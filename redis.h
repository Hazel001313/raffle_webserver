#ifndef SERVER_REDIS_H_
#define SERVER_REDIS_H_

#include <stdio.h>
#include <hiredis/hiredis.h>
#include <string>

class Redis
{
public:
    Redis();
    Redis(const char *host, int port);
    ~Redis();
    void flush();
    void increby1(const std::string key);
    std::string get(const std::string key);
    void set(const std::string key, const std::string value);
    void sadd(const std::string key, const std::string member);
    void hset(const std::string key, const std::string field, const std::string value);
    bool sismember(const std::string key, const std::string member);
    bool hexists(const std::string key, const std::string field);
    bool hsetnx(const std::string key, const std::string field, const std::string value);
    std::string hget(const std::string key, const std::string field);
    std::string spop(const std::string key);
    void multi();
    bool exec();
    void watch(const std::string key);
    void unwatch();

private:
    redisContext *redisconnect_;
    void checkreturn(void *context_or_reply);
};

#endif //SERVER_REDIS_H_