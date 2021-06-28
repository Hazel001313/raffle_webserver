#ifndef SERVER_H_
#define SERVER_H_

#include <sys/epoll.h>

#include "http_conn.h"
#include "threadpool.h"
#include "router.h"

const int MAX_EVENT_NUMBER = 10000;

class server
{
public:
    ~server()
    {
        close(epollfd_);
        close(listenfd_);
        delete pool_;
    }
    static server &get_instance()
    {
        static server server_;
        return server_;
    }

    void init(int port, int ticket_number, http_conn *m_http_conn, int fd_num);
    void eventloop();

private:
    server(){};
    server(server &) = delete;
    server &operator=(const server &) = delete;
    int port_;
    int listenfd_;
    int stopservice_;
    int epollfd_;
    threadpool<http_conn> *pool_;
    http_conn *http_conn_;
    int fd_num_;

    epoll_event events_[MAX_EVENT_NUMBER];
};

#endif //SERVER_H_