#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_
 
#include <iostream>
#include <queue>
#include <exception>
#include <errno.h>
#include <pthread.h>
#include "Locker.hpp"

using namespace std;
 
template<class T>
class threadpool{
private:
    int thread_number;  //线程池的线程数
    //int max_task_number;  //任务队列中的最大任务数
    pthread_t *all_threads;   //线程数组
    queue<T *> task_queue; //任务队列
    mutex_locker queue_mutex_locker;  //互斥锁
    //sem_locker queue_sem_locker;   //信号量
    cond_locker queue_cond_locker; //条件变量
    bool is_stop; //是否结束线程

public:
    threadpool(int thread_num = 20);
    ~threadpool();
    bool append_task(T *task);  //添加任务
    void start();              //线程池开启
    void stop();               //线程池关闭

private:
    //线程运行的函数。执行run()函数
    static void *worker(void *arg);
    void run();
    T *getTask();   //获取任务
};
 
template <class T>
threadpool<T>::threadpool(int thread_num): thread_number(thread_num), is_stop(false), all_threads(NULL){       
    if(thread_num <= 0)
	    cout << "threadpool can't init because thread_number = 0" << endl;
 
    all_threads = new pthread_t[thread_number];

    if(all_threads == NULL)
	    cout << "can't init threadpool because thread array can't new" << endl;
}
 
template <class T>
threadpool<T>::~threadpool(){
    delete []all_threads;
    stop();
}
 
template <class T>
void threadpool<T>::stop(){ //线程池停止
        is_stop = true;
        //queue_sem_locker.add();
        queue_cond_locker.broadcast();
}

//线程池启动
template <class T>
void threadpool<T>::start()  {
    for(int i = 0; i < thread_number; ++i){
	    if(pthread_create(all_threads + i, NULL, worker, this) != 0){ //创建线程失败，清除成功申请的资源并抛出异常
	        delete []all_threads;
	        throw std::exception();
	    }
	    if(pthread_detach(all_threads[i])){ //将线程设置为脱离线程，失败则清除成功申请的资源并抛出异常
	        delete []all_threads;
	        throw std::exception();
	    }
    }
}

//添加任务进入任务队列
template <class T>
bool threadpool<T>::append_task(T *task){   
    queue_mutex_locker.mutex_lock();
    
    bool is_signal = task_queue.empty();

    task_queue.push(task);

    queue_mutex_locker.mutex_unlock();
    
    if(is_signal)
        queue_cond_locker.signal();
    
    return true;
}
 
template <class T>
void *threadpool<T>::worker(void *arg){
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}
 
template <class T>
T *threadpool<T>::getTask(){
    T *task = NULL;
    queue_mutex_locker.mutex_lock();
    if(!task_queue.empty())
    {
        task = task_queue.front();
        task_queue.pop();
    }
    queue_mutex_locker.mutex_unlock();
    return task;
}
 
template <class T>
void threadpool<T>::run(){
    while(!is_stop){
        T *task = getTask();
        if(task == NULL)  //队列为空，等待
            queue_cond_locker.wait();
        else              //执行任务
            task->doit();
    }
}
 
#endif
