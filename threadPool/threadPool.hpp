#pragma once
#include <pthread.h>
#include <vector>
#include <queue>
#include <iostream>
#include "LockGurad.hpp"
#include "Thread.hpp"
#include <unistd.h>

static const int N = 5;

template<class T>
class threadPool
{
private:
    //因为线程池是单例模式，所以要把构造函数放到private中，拷贝构造和赋值重载delete了
    threadPool(int num = N):_num(num)
    {
        pthread_mutex_init(&_lock,nullptr);
        pthread_cond_init(&_cond,nullptr);
        pthread_mutex_init(&_push_lock,nullptr);
    }
    threadPool(const threadPool<T>& tp) = delete;
    void operator=(const threadPool<T>& tp) = delete;
public:
    static threadPool<T>* getinstance()
    {
        if(instance == nullptr)
        {
            LockGuard lockguard(&instance_lock);
            if(instance == nullptr)
            {
                instance = new threadPool<T>();
                instance->init();
                instance->start();
            }
        }
        
        return instance;
    }

    pthread_mutex_t* getlock()
    {
        return &_lock;
    }
    
    void threadwait()
    {
        pthread_cond_wait(&_cond,&_lock);
    }

    void threadWakeUp()
    {
        pthread_cond_signal(&_cond);
    }

    bool isEmpty()
    {
        return _tasks.empty();
    }

    T popTask()
    {
        T task = _tasks.front();
        _tasks.pop();
        return task;
    }

    void pushTask(const T& task)
    {
        LockGuard lockguard(&_push_lock);
        _tasks.push(task);
        threadWakeUp();
    }

    static void threadRoutine(void* args)
    {
        threadPool<T>* tp = static_cast<threadPool<T>* >(args);//获取this指针
        while(true)
        {
            T t;
            {
                LockGuard lockguard(instance->getlock());
                while(tp->isEmpty())
                {
                    tp->threadwait();//任务列表为空就等待
                }
                t = tp->popTask();//获取任务t
            }
            t();//利用任务的仿函数t()解决其问题
            std::cout << "thread handler done, result: " << t.formatRes() << std::endl;
        }
    }

    void init()
    {
        for(int i = 0;i<_num;++i)
        {
            _threads.push_back(Thread(i,threadRoutine,this));
        }
    }

    void start()
    {
        for(auto& t :_threads)
        {
            t.run();
        }
    }

    void check()
    {
        for(auto& t : _threads)
        {
            std::cout<<t.threadname()<<"is runnning"<<std::endl;
        }
    }

    ~threadPool()
    {
        for(auto& t : _threads)
        {
            t.join();
        }
        pthread_mutex_destroy(&_lock);
        pthread_cond_destroy(&_cond);
    }

private:
    std::vector<Thread> _threads;//存储线程池容器
    int _num;//线程数量
    std::queue<T> _tasks;//任务队列
    pthread_mutex_t _lock;//push和pop是的锁
    pthread_mutex_t _push_lock;
    pthread_cond_t _cond;//条件变量
    static pthread_mutex_t instance_lock;//instance的锁
    static threadPool<T>* instance;//其实就是this指针，为了放进static函数内
};

template<class T>
pthread_mutex_t threadPool<T>::instance_lock = PTHREAD_MUTEX_INITIALIZER;//static的锁需要在类外进行初始化

template<class T>
threadPool<T>* threadPool<T>::instance = nullptr;//同上