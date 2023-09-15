#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>

#include "locker.h"

template< typename T >
class threadpool{
public:
    /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许存在里请求*/
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    /*往请求队列中添加任务*/
    bool append(T* requests);
private:
    static void* worker(void* arg);
    void run();

private:
    //线程池中的线程数
    int m_thread_number;
    //请求队列中的最大请求数
    int m_max_requests;
    //线程池，描述线程池的数组
    pthread_t* m_threads;
    //请求队列
    std::list< T* > m_workqueue;
    //保护请求队列的互斥锁
    locker m_queuelocker;
    sem m_queuestat;
    //是否结束线程
    bool m_stop;
};


#endif