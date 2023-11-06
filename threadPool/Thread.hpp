#pragma once

#include <pthread.h>
#include <string>
#include <iostream>

class Thread
{
public:
    typedef enum
    {
        New = 0,
        RUNNING,
        EXITED
    } ThreadStatus;
    
    typedef void (*func_t)(void *);

    Thread(int num, func_t func, void *args) : _tid(0), _func(func), _args(args), _status(New)
    {
        char name[64];
        snprintf(name, 64, "thread-%d", num);
        _name = name;
    }

    int status()
    {
        return _status;
    }

    std::string threadname()
    {
        return _name;
    }

    pthread_t threadid()
    {

        if (_status == RUNNING)
        {
            return _tid;
        }

        return 0;
    }

    static void *runhelper(void *args)
    {
        Thread *ts = static_cast<Thread *>(args);
        (*ts)();
        return nullptr;
    }

    void operator()()
    {
        if (_func != nullptr)
            _func(_args);
    }

    void run()
    {
        int n = pthread_create(&_tid, nullptr, runhelper, this);
        if (n != 0)
            exit(1);

        _status = RUNNING;
    }

    void join()
    {
        int n = pthread_join(_tid, nullptr);
        if (n != 0)
        {
            std::cerr << "thraad join error - " << _name << std::endl;
        }

        _status = EXITED;
    }

    ~Thread() {}

private:
    pthread_t _tid;       // 线程tid
    std::string _name;    // 线程名(其实就是线程编号)
    func_t _func;         // 回调函数
    void *_args;          // 存储传给线程的参数
    ThreadStatus _status; // 线程状态
};