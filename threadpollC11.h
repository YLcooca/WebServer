#pragma once
#include <iostream>
#include "locker.h"
#include <memory>
#include <mutex>
#include <semaphore.h>
#include <list>
#include <thread>
#include <exception>
#include <fstream>

using namespace std;

template <typename T>
class threadpoll
{
// friend fstream fs("log.txt", ios::app | ios::in);   
public:
    threadpoll(int thread_number = 8, int max_requests = 10000);
    ~threadpoll();
    bool append(T *request);

private:
    int m_thread_number;  // 线程数量
    int m_max_requests;   // 最大请求数量
    thread *pool;         // 线程数组
    list<T *> workqueue;  // 请求队列
    mutex workqueue_lock; // 请求队列的锁
    bool m_stop;
    sem m_queuestat; // 是否有任务需要处理
    fstream fs;
    // void worker();
    void run();
};

template <typename T>
threadpoll<T>::threadpoll(int thread_number, int max_requests)
    : m_thread_number(thread_number), m_max_requests(max_requests),
      pool(nullptr), m_stop(false), fs(fstream("log.txt", ios::app | ios::in))
{
    if (thread_number < 1 || max_requests < 1)
    {
        throw exception();
    }
    pool = new thread[m_thread_number];
    // 创建线程池
    for (int i = 0; i < m_thread_number; ++i)
    {
        pool[i] = thread(&threadpoll::run, this); // 创建线程
        pool[i].detach();                         // 设置为分离状态
        cout << "create thread " << pool[i].get_id() << endl;
    }
}

template <typename T>
threadpoll<T>::~threadpoll()
{
    delete[] pool;
    m_stop = true;
}

template <typename T>
bool threadpoll<T>::append(T *request)
{
    unique_lock<mutex> uq(workqueue_lock); // 获得锁
    if (workqueue.size() > m_max_requests)
    {
        // 队列满了，添加失败
        return false;
    }
    // 未满，开始添加
    workqueue.push_back(request);
    uq.unlock();        // 释放锁
    m_queuestat.post(); // 信号量加1，通知线程处理
    return true;
}

template <typename T>
void threadpoll<T>::run()
{
    while (!m_stop)
    {
        m_queuestat.wait(); // 请求队列为空则阻塞在此
        unique_lock<mutex> uq(workqueue_lock);
        if (workqueue.empty())
        {
            // 有可能虚假唤醒
            uq.unlock();
            continue;
        }

        T *request = workqueue.front(); // 取任务
        workqueue.pop_front();
        
        uq.unlock();
        if (!request)
        {
            continue; // 取失败了
        }
        fs << this_thread::get_id() << " process\n";
        request->process();
    }
}