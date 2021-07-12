#include <stdlib.h>
#include "server.h"
#include "raffle.h"
#include "http_conn.h"
#include "router.h"
#include "conn_pool.h"
#include "redis.h"

#define CONN_COUNT 600
const int MAX_FD = 65536;

const char *ok_win_form = "Congratulation! You've got the ticket!";
const char *ok_lose_form = "Sorry... But you can watch on line!\n";
const char *invalid_staff_id = "Invalid id\n";
const char *finish_raffle = "Finish!\n";

int main(int argc, char *argv[])
{

    int port = 7777;
    std::string redis_ip="127.0.0.1";
    int redis_port=6379;
    int server_id=2;
    int ticket_num=40000;

    int opt;
    const char *str = "p:r:t:i:n:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            port = atoi(optarg);
            break;
        }
        case 'r':
        {
            redis_ip = optarg;
            break;
        }
        case 't':
        {
            redis_port = atoi(optarg);
            break;
        }
        case 'i':
        {
            server_id = atoi(optarg);
            break;
        }
        case 'n':
        {
            ticket_num = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }


    server &m_raffleserver = server::get_instance();
    conn_pool<Redis> &m_conn_pool = conn_pool<Redis>::get_instance();

    http_conn *m_http_conn;
    raffle *m_raffle;
    router *m_router;

    m_http_conn = new http_conn[MAX_FD];

    try
    {
        m_conn_pool.init(redis_ip, redis_port, CONN_COUNT);
        m_raffle = new raffle(m_conn_pool, "stafflist.csv", ticket_num, server_id);
        m_router = new router;
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

    m_router->addRoute("/", [m_raffle, server_id](http_conn *http_connect) -> http_conn::HTTP_CODE
                       {
                           auto id = http_connect->query_value.find("id");
                           if (id == http_connect->query_value.end())
                           {
                               m_raffle->webbench_test();
                           }
                           else if (!m_raffle->ismember(id->second))
                           {
                               sprintf(http_connect->m_content_buf, "%s (From Server No. %d)\n", invalid_staff_id, server_id);
                           }
                           else
                           {
                               int ticket_id = m_raffle->draw_ticket_(id->second);
                               if (ticket_id > 0)
                               {
                                   sprintf(http_connect->m_content_buf, "%s Your ticket number is %d \n(From Server No. %d)\n", ok_win_form, ticket_id, server_id);
                               }
                               if (ticket_id == 0)
                               {
                                   sprintf(http_connect->m_content_buf, "%s (From Server No. %d)\n", ok_lose_form, server_id);
                               }
                           }
                           return http_conn::GET_REQUEST;
                       });

    m_router->addRoute("/fin", [m_raffle, server_id](http_conn *http_connect) -> http_conn::HTTP_CODE
                       {
                           if (http_connect->query_value["passwd"] == "123")
                           {
                               m_raffle->finish();
                               sprintf(http_connect->m_content_buf, "%s (From Server No. %d)\n", finish_raffle, server_id);
                               return http_conn::GET_REQUEST;
                           }
                           else
                               return http_conn::UNAUTHORIZED;
                       });

    m_router->addRoute("*", [m_raffle](http_conn *http_connect) -> http_conn::HTTP_CODE
                       {
                           const int FILENAME_LEN = 200;
                           char filedir[FILENAME_LEN];
                           char *dir = getcwd(filedir, sizeof(filedir));

                           if (strlen(filedir) + strlen(http_connect->m_url) >= FILENAME_LEN)
                               return http_conn::BAD_REQUEST;

                           strcat(filedir, http_connect->m_url);
                           //printf("filedir: %s\n",filedir);
                           if (stat(filedir, &http_connect->m_file_stat) < 0)
                           {
                               return http_conn::NO_RESOURCE;
                           }
                           if (!(http_connect->m_file_stat.st_mode & S_IROTH))
                           {
                               return http_conn::FORBIDDEN_REQUEST;
                           }
                           if (S_ISDIR(http_connect->m_file_stat.st_mode))
                           {
                               return http_conn::BAD_REQUEST;
                           }
                           int fd = open(filedir, O_RDONLY);
                           http_connect->m_file_address = (char *)mmap(0, http_connect->m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
                           close(fd);
                           return http_conn::FILE_REQUEST;
                       });

    http_conn::m_router = m_router;

    m_raffleserver.init(port, ticket_num, m_http_conn, MAX_FD);
    m_raffleserver.eventloop();

    delete m_router;
    delete m_raffle;
    delete[] m_http_conn;

    return 0;
}
