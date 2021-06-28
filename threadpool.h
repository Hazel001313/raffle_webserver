#ifndef SERVER_THREADPOOL_H
#define SERVER_THREADPOOL_H

#include <queue>
#include <cstdio>
#include <exception>
#include <iostream>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <memory>

#include "redis.h"

// 线程池类，将它定义为模板类是为了代码复用，模板参数T是任务类
template <typename T>
class threadpool
{
public:
    /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
    threadpool(int thread_number = 600, int max_requests = 1000000);
    ~threadpool();
    bool append(T *request);

private:
    //工作线程运行的函数，它不断从工作队列中取出任务并执行
    static void *worker(void *arg);
    void run();

private:
    // 线程的数量
    int m_thread_number;

    // 描述线程池的数组，大小为m_thread_number
    std::vector<std::shared_ptr<std::thread>> m_threads;

    // 请求队列中最多允许的、等待处理的请求的数量
    int m_max_requests;

    // 请求队列
    std::queue<T *> m_workqueue;

    // 保护请求队列的互斥锁
    std::mutex m_mutex_;

    // 指示任务队列中放入任务的条件变量
    std::condition_variable m_cv_;

    // 是否结束线程
    bool m_stop;
};

template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests) : m_thread_number(thread_number), m_max_requests(max_requests),
                                                                 m_stop(false)
{

    if (thread_number <= 0)
    {
        throw std::invalid_argument("Invalid thread number");
    }
    if (max_requests <= 0)
    {
        throw std::invalid_argument("Invalid max_requests");
    }

    // 创建thread_number 个线程，并将他们设置为脱离线程。
    for (int i = 0; i < thread_number; ++i)
    {
        //printf( "create the %dth thread\n", i);

        m_threads.push_back(std::make_shared<std::thread>([this]()
                                                          {
                                                              std::unique_lock<std::mutex> locker(m_mutex_, std::defer_lock);
                                                              while (!m_stop)
                                                              {
                                                                  locker.lock();
                                                                  m_cv_.wait(locker, [this]
                                                                             { return !m_workqueue.empty(); });
                                                                  T *request = m_workqueue.front();
                                                                  m_workqueue.pop();
                                                                  locker.unlock();
                                                                  if (!request)
                                                                  {
                                                                      continue;
                                                                  }
                                                                  try
                                                                  {
                                                                      request->process();
                                                                  }
                                                                  catch (const char *msg)
                                                                  {
                                                                      std::cout << "thread " << std::this_thread::get_id() << ": " << msg << std::endl;
                                                                  }
                                                              }
                                                          }));

        m_threads[i]->detach();
    }
}

template <typename T>
threadpool<T>::~threadpool()
{
    m_stop = true;
}

template <typename T>
bool threadpool<T>::append(T *request)
{
    // 操作工作队列时一定要加锁，因为它被所有线程共享。
    std::unique_lock<std::mutex> locker(m_mutex_);
    if (m_workqueue.size() > m_max_requests)
    {
        locker.unlock();
        return false;
    }
    m_workqueue.emplace(request);
    locker.unlock();
    m_cv_.notify_one();
    return true;
}

#endif
