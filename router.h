#ifndef SERVER_ROUTER_H_
#define SERVER_ROUTER_H_

#include <functional>
#include <unordered_map>

#include "http_conn.h"

class router
{
public:
    void addRoute(std::string url, std::function<http_conn::HTTP_CODE(http_conn *)> func)
    {
        router[url] = func;
    }
    http_conn::HTTP_CODE handle(http_conn *http_connect)
    {
        if (router.find(http_connect->m_url) == router.end())
        {
            return router["*"](http_connect);
        }
        else
        {
            return router[http_connect->m_url](http_connect);
        }
    }

private:
    std::unordered_map<std::string, std::function<http_conn::HTTP_CODE(http_conn *)>> router;
};

#endif //SERVER_ROUTER_H