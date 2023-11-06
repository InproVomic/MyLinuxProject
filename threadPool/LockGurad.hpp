#pragma once
#include <pthread.h>
#include <iostream>

class Mutex
{
public:
    Mutex(pthread_mutex_t* mutex):_mutex(mutex)
    {}

    void lock()
    {
        pthread_mutex_lock(_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(_mutex);
    }

    ~Mutex()
    {}
private:
    pthread_mutex_t* _mutex;
};

class LockGuard
{
public:
    LockGuard(pthread_mutex_t* mutex):_mutex(mutex)
    {
        _mutex.lock();
    }

    ~LockGuard()
    {
        _mutex.unlock();
    }
    
private:
    Mutex _mutex;
};