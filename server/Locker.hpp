#ifndef _LOCKER_H_
#define _LOCKER_H_

#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;
 
/*信号量*/
class sem_locker
{
private:
    sem_t m_sem;	//信号量类型
 
public:
    //初始化信号量
    sem_locker(){
	if(sem_init(&m_sem, 0, 0) != 0)
		cout << "sem init error" << endl;
    }

    //销毁信号量
    ~sem_locker(){
	    sem_destroy(&m_sem);
    }
 
    //等待信号量
    bool wait(){
	    return sem_wait(&m_sem) == 0;
    }
    //添加信号量
    bool add(){
	    return sem_post(&m_sem) == 0;
    }
};
 
 
/*互斥锁*/
class mutex_locker{
private:
    pthread_mutex_t m_mutex;
 
public:
     //初始化锁
    mutex_locker(){
    	if(pthread_mutex_init(&m_mutex, NULL) != 0)
		cout << "mutex init error" << endl;
    }

    //销毁锁
    ~mutex_locker(){
	    pthread_mutex_destroy(&m_mutex);
    }

    //加锁
    bool mutex_lock(){
	    return pthread_mutex_lock(&m_mutex) == 0;
    }

    //解锁
    bool mutex_unlock(){
	    return pthread_mutex_unlock(&m_mutex) == 0;
    }
};
 
/*条件变量*/
class cond_locker
{
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
 
public:
    // 初始化mutex和cond
    cond_locker(){
	    if(pthread_mutex_init(&m_mutex, NULL) != 0)
		    cout << "mutex init error" << endl;

	    if(pthread_cond_init(&m_cond, NULL) != 0) //如果条件变量初始化失败，锁就用不上了，所以要销毁掉
	    {   
	        pthread_mutex_destroy(&m_mutex);
		    cout << "cond init error" << endl;
	    }
    }

    // 销毁mutex和cond
    ~cond_locker(){
	    pthread_mutex_destroy(&m_mutex);
	    pthread_cond_destroy(&m_cond);
    }

    //等待条件变量
    bool wait(){
	    int ans = 0;
	    pthread_mutex_lock(&m_mutex);
	    ans = pthread_cond_wait(&m_cond, &m_mutex);
	    pthread_mutex_unlock(&m_mutex);
	    return ans == 0;
    }

    //唤醒等待条件变量的线程
    bool signal(){
	    return pthread_cond_signal(&m_cond) == 0;
    }
 
    //唤醒all等待条件变量的线程
    bool broadcast(){
    	return pthread_cond_broadcast(&m_cond) == 0;
    }
};
 
#endif
