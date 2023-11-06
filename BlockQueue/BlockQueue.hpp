#pragma once 
#include <iostream>
#include <pthread.h>
#include<queue>
#include <functional>

const int gcap = 5;

template<class T>
class BlockQueue
{
public:
    BlockQueue(const int cap = gcap):_cap(cap)
    {
        pthread_mutex_init(&_mutex,nullptr);
        pthread_cond_init(&_productorCond,nullptr);
        pthread_cond_init(&_consumerCond,nullptr);
    }

    bool isFull()
    {
        return _cap == _q.size();
    }

    bool isEmpty()
    {
        return _q.empty();
    }

    void push(const T& in)
    {
        pthread_mutex_lock(&_mutex);

        while(isFull())
        {
            pthread_cond_signal(&_consumerCond);
            pthread_cond_wait(&_productorCond,&_mutex);
        }

        _q.push(in);
        pthread_cond_signal(&_consumerCond);
        pthread_mutex_unlock(&_mutex);
    }

    void pop(T* out)
    {
        pthread_mutex_lock(&_mutex);

        while(isEmpty())
        {
            pthread_cond_signal(&_productorCond);
            pthread_cond_wait(&_consumerCond,&_mutex);
        }

        *out = _q.front();
        _q.pop();

        pthread_cond_signal(&_productorCond);
        pthread_mutex_unlock(&_mutex);
    }

    ~BlockQueue()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_productorCond);
        pthread_cond_destroy(&_consumerCond);
    }

private:
    std::queue<T> _q;//这个是存储任务队列
    pthread_cond_t _productorCond;//生产者条件变量
    pthread_cond_t _consumerCond;//消费者条件变量
    pthread_mutex_t _mutex;
    int _cap;//这个是队列最大空间
};
