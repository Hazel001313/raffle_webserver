#ifndef SERVER_DB_POOL_H_
#define SERVER_DB_POOL_H_

#include <condition_variable>
#include <exception>
#include <list>
#include <memory>
#include <mutex>
#include <string>



//database connection pool, singleton.
template <typename DB>
class conn_pool
{
public:
    static conn_pool &get_instance()
    {
        static conn_pool db_pool_;
        return db_pool_;
    }
    //initiate databasepool
    void init(std::string ip, int port, int context_num = 50)
    {
        if (context_num <= 0)
            throw std::invalid_argument("conn_pool size<=0");
        ip_ = ip;
        port_ = port;
        pool_size_ = context_num;

        for (int i = 0; i < pool_size_; i++)
        {
            connection_vec_.push_back(std::make_shared<DB>(ip_.c_str(), port_));
            ++idle_context_num_;
        }
    }
    //return one available DB context
    std::shared_ptr<DB> get()
    {
        // if(pool_size_<=0){
        //     throw std::invalid_argument("conn_pool size<=0");
        // }

        std::unique_lock<std::mutex> lock(mutex_);
        condition_v_.wait(lock, [this]
                          { return idle_context_num_ > 0; });

        auto rds = connection_vec_.front();
        connection_vec_.pop_front();
        --idle_context_num_;
        lock.unlock();
        return rds;
    }
    //release one DB context
    bool release(std::shared_ptr<DB> rds)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connection_vec_.push_back(rds);
        ++idle_context_num_;
        lock.unlock();
        condition_v_.notify_one();

        return true;
    }
    ~conn_pool(){};

private:
    conn_pool()
    {
        pool_size_ = 0;
        idle_context_num_ = 0;
    }

    std::string ip_;
    int port_;

    int pool_size_;
    int idle_context_num_;

    std::mutex mutex_;
    std::condition_variable condition_v_;

    std::list<std::shared_ptr<DB>> connection_vec_;
};

template <typename DB>
class conn_guard
{
public:
    conn_guard(std::shared_ptr<DB> con) : con_(con) {}
    ~conn_guard()
    {
        conn_pool<DB>::get_instance().release(con_.lock());
    }

private:
    std::weak_ptr<DB> con_;
};

#endif