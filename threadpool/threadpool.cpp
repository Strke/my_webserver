#include "threadpool.h"

template< typename T >
threadpool< T >::threadpool(int thread_number, int max_requests):
m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false), m_threads(NULL){
    /*如果线程池容纳大小或者队列内最大请求小于0，则抛出异常*/
    if((thread_number <= 0) || (max_requests <= 0)){
        throw std::exception();
    }
    /*初始化线程池*/
    m_threads = new pthread_t[m_thread_number];
    
    /*初始化失败则抛出异常*/
    if(!m_threads){
        throw std::exception();
    }
    /*创建thread_number个线程，并将他们设置为脱离线程*/
    for(int i = 0; i < thread_number; i ++){
        /*如果创建线程失败，则删除所有线程*/
        if(pthread_create(m_threads + 1, NULL, worker, this) != 0){
            delete [] m_threads;
            throw std::exception();
        }
        /*设置脱离线程其资源可以由系统自动进行回收*/
        if(pthread_detach( m_threads[i] )){
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template< typename T >
threadpool< T >::~threadpool(){
    delete [] m_threads;
    m_stop = true;
}

/*给请求队列添加任务*/
template< typename T > 
bool threadpool< T >::append(T* request){
    /*加锁避免多个线程同时访问*/
    m_workqueue.lock();
    if(m_workqueue.size() > m_max_requests){
        m_workqueue.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_workqueue.unlock();
    m_queuestat.post();
    return true;
}
template< typename T >
void* threadpool< T >::worker(void* arg){
    threadpool* pool = (threadpool*) arg;
    pool -> run();
    return pool;
}


template< typename T >
void threadpool< T >::run(){
    while(!m_stop){
        m_queuestat.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty()){
            m_workqueue.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(!request) continue;
        request -> process();
    }
}