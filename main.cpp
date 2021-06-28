#include <stdlib.h>
#include "server.h"
#include "raffle.h"
#include "http_conn.h"
#include "router.h"
#include "conn_pool.h"
#include "redis.h"

#define CONN_COUNT 600
const int MAX_FD = 65536;

int main(int argc, char *argv[])
{

    if (argc <= 3)
    {
        printf("usage: %s port_number ticket_number server_id\n", basename(argv[0]));
        return 1;
    }

    int port = atoi(argv[1]);
    int ticket_number = atoi(argv[2]);
    int server_id = atoi(argv[3]);

    server &m_raffleserver = server::get_instance();
    conn_pool<Redis> &m_conn_pool = conn_pool<Redis>::get_instance();

    http_conn *m_http_conn;
    raffle *m_raffle;
    router *m_router;

    m_http_conn = new http_conn[MAX_FD];

    try
    {
        m_conn_pool.init("9.135.13.170", 6379, CONN_COUNT);
        m_raffle = new raffle(m_conn_pool, "stafflist.csv", ticket_number, server_id);
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

    m_router->addRoute("/", [m_raffle](http_conn *http_connect) -> http_conn::HTTP_CODE
                       {
                           auto id = http_connect->query_value.find("id");
                           if (id == http_connect->query_value.end())
                           {
                               m_raffle->webbench_test();
                           }
                           else if (!m_raffle->ismember(id->second))
                           {
                               http_connect->m_ticket_id = -1;
                           }
                           else
                           {
                               http_connect->m_ticket_id = m_raffle->draw_ticket_(id->second);
                           }
                           http_connect->m_request = http_conn::REQUEST_CODE::DRAW_TICKET;
                           return http_conn::GET_REQUEST;
                       });

    m_router->addRoute("/fin", [m_raffle](http_conn *http_connect) -> http_conn::HTTP_CODE
                       {
                           if (http_connect->query_value["passwd"] == "123")
                           {
                               m_raffle->finish();
                               http_connect->m_request = http_conn::REQUEST_CODE::FINISH;
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
    http_conn::m_server_id = server_id;

    m_raffleserver.init(port, ticket_number, m_http_conn, MAX_FD);
    m_raffleserver.eventloop();

    delete m_router;
    delete m_raffle;
    delete[] m_http_conn;

    return 0;
}
