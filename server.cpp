#include "server.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <exception>
#include <iostream>

#include "http_conn.h"

#define THREAD_COUNT 600

extern void addfd(int epollfd, int fd, bool one_shot);
extern void removefd(int epollfd, int fd);

void addsig(int sig, void(handler)(int))
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void server::init(int port, int ticket_number, http_conn *m_http_conn, int fd_num)
{

    port_ = port;

    stopservice_ = false;
    http_conn_ = m_http_conn;
    fd_num_ = fd_num;

    addsig(SIGPIPE, SIG_IGN);

    try
    {
        //initiate redis_pool connection
        pool_ = new threadpool<http_conn>(THREAD_COUNT);
    }
    catch (std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
        exit(1);
    }
    catch (const char *msg)
    {
        std::cout << "Initiate server error: " << msg << std::endl;
        exit(1);
    }

    listenfd_ = socket(PF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in saddr;
    bzero(&saddr, sizeof(saddr));
    saddr.sin_port = htons(port_);
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = 0;
    ret = bind(listenfd_, (struct sockaddr *)&saddr, sizeof(saddr));
    assert(ret >= 0);

    ret = listen(listenfd_, 13);
    assert(ret >= 0);

    epollfd_ = epoll_create(100);
    addfd(epollfd_, listenfd_, false);

    http_conn::m_epollfd = epollfd_;
}

void server::eventloop()
{
    while (!stopservice_)
    {
        int number = epoll_wait(epollfd_, events_, MAX_EVENT_NUMBER, -1);
        if ((number < 0) && (errno != EINTR))
        {
            printf("epoll failure\n");
            break;
        }

        for (int i = 0; i < number; i++)
        {

            int sockfd = events_[i].data.fd;
            if (sockfd == listenfd_)
            {

                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd_, (struct sockaddr *)&client_address, &client_addrlength);

                if (connfd < 0)
                {
                    printf("errno is: %d\n", errno);
                    continue;
                }

                if (http_conn::m_user_count >= fd_num_)
                {
                    close(connfd);
                    continue;
                }
                http_conn_[connfd].init(connfd, client_address);
            }
            else if (events_[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {

                http_conn_[sockfd].close_conn();
            }
            else if (events_[i].events & EPOLLIN)
            {
                pool_->append(&http_conn_[sockfd]);
            }
        }
    }
}