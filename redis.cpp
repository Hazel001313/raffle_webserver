#include "redis.h"
#include <cstdlib>

void Redis::checkreturn(void *context_or_reply)
{
    if (context_or_reply == NULL || redisconnect_->err)
    {
        if (redisconnect_)
        {
            throw(redisconnect_->errstr);
        }
        else
        {
            throw("Can't allocate redis context");
        }
    }
}
Redis::Redis() : redisconnect_(nullptr)
{
}

Redis::Redis(const char *host, int port)
{
    redisconnect_ = redisConnect(host, port);
    checkreturn(redisconnect_);
}

Redis::~Redis()
{
    redisFree(redisconnect_);
    redisconnect_ = nullptr;
}

void Redis::increby1(const std::string key)
{
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "INCRBY %s 1", key.c_str());
    checkreturn(reply);
    freeReplyObject(reply);
}

void Redis::flush()
{
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "FLUSHDB");
    checkreturn(reply);
    freeReplyObject(reply);
    return;
}

std::string Redis::get(const std::string key)
{
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "GET %s", key.c_str());
    checkreturn(reply);
    std::string s = reply->str;
    freeReplyObject(reply);
    return s;
}

void Redis::set(const std::string key, const std::string value)
{
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "SET %s %s", key.c_str(), value.c_str());
    checkreturn(reply);
    freeReplyObject(reply);
    return;
}

void Redis::hset(const std::string key, const std::string field, const std::string value)
{
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
    checkreturn(reply);
    freeReplyObject(reply);
    return;
}

void Redis::sadd(const std::string key, const std::string member)
{
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "SADD %s %s", key.c_str(), member.c_str());
    checkreturn(reply);
    freeReplyObject(reply);
    return;
}

bool Redis::sismember(const std::string key, const std::string member)
{
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "SISMEMBER %s %s", key.c_str(), member.c_str());
    int ret = reply->integer;
    freeReplyObject(reply);
    return (ret == 1);
}

bool Redis::hexists(const std::string key, const std::string field)
{
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "HEXISTS %s %s", key.c_str(), field.c_str());
    checkreturn(reply);
    int i = reply->integer;
    freeReplyObject(reply);
    return (i == 1);
}

bool Redis::hsetnx(const std::string key, const std::string field, const std::string value)
{
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "HSETNX %s %s %s", key.c_str(), field.c_str(), value.c_str());
    checkreturn(reply);
    int i = reply->integer;
    freeReplyObject(reply);
    return (i == 1);
}

std::string Redis::hget(const std::string key, const std::string field)
{
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "HGET %s %s", key.c_str(), field.c_str());
    checkreturn(reply);
    std::string s;
    if (reply->type == REDIS_REPLY_NIL)
        s = "0";
    else
        s = reply->str;
    freeReplyObject(reply);
    return s;
}

std::string Redis::spop(const std::string key)
{
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "SPOP %s", key.c_str());
    checkreturn(reply);
    std::string s;
    if (reply->type == REDIS_REPLY_NIL)
        s = "0";
    else
        s = reply->str;
    freeReplyObject(reply);
    return s;
}

void Redis::multi(){
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "MULTI");
    checkreturn(reply);
    freeReplyObject(reply);
    return;
}
bool Redis::exec(){
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "EXEC");
    checkreturn(reply);
    bool ret=false;
    if (reply->type != REDIS_REPLY_NIL)
    ret=true;
    freeReplyObject(reply);
    return ret;
}
void Redis::watch(const std::string key){
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "WATCH %s", key.c_str());
    checkreturn(reply);
    freeReplyObject(reply);
    return;
}
void Redis::unwatch(){
    redisReply *reply = (redisReply *)redisCommand(redisconnect_, "UNWATCH");
    checkreturn(reply);
    freeReplyObject(reply);
    return;
}