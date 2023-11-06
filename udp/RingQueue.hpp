#pragma once

#include<pthread.h>
#include<semaphore.h>
#include <iostream>
#include <queue>

static const int N = 5;

template<class T>
class RingQueue
{
private:
    void P(sem_t &s)
    {
        sem_wait(&s);
    }

    void V(sem_t &s)
    {
        sem_post(&s);
    }

    void Lock(pthread_mutex_t &m)
    {
        pthread_mutex_lock(&m);
    }

    void Unlock(pthread_mutex_t& m)
    {
        pthread_mutex_unlock(&m);
    }

public:
    RingQueue(int num = N):_ring(num),_cap(num)
    {
        pthread_mutex_init(&_c_mutex,nullptr);
        pthread_mutex_init(&_p_mutex,nullptr);
        sem_init(&_data_sem,0,0);
        sem_init(&_space_sem,0,5);
        _c_step = _p_step = 0;
    }

    ~RingQueue()
    {
        pthread_mutex_destroy(&_c_mutex);
        pthread_mutex_destroy(&_p_mutex);
        sem_destroy(&_data_sem);
        sem_destroy(&_space_sem);
    }

    void push(const T& in)//生产者往队列中放数据
    {
        P(_space_sem);//减少空间信号量
        Lock(_p_mutex);
        _ring[_p_step++] = in;//把数据放进队列中
        _p_step%=_cap;//确保生产者位置不会超过容量
        Unlock(_p_mutex);
        V(_data_sem);//增加数据信号量
    }

    void pop(T* out)//消费者往队列中取数据
    {        
        P(_data_sem);
        Lock(_c_mutex);
        *out = _ring[_c_step++];
        _c_step%=_cap;
        Unlock(_c_mutex);
        V(_space_sem);
    }

private:
    std::vector<T> _ring;//环形队列
    int _cap;//环形队列可放的数据量
    pthread_mutex_t _c_mutex;//消费者锁
    pthread_mutex_t _p_mutex;//生产者锁
    int _c_step;//消费者位置
    int _p_step;//生产者位置
    sem_t _data_sem;//数据资料，只有消费者关系
    sem_t _space_sem;//空间资料，只有生产者关心
};
