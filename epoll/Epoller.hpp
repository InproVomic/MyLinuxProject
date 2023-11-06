#pragma once
#include <sys/epoll.h>
#include <string.h>
#include "log.hpp"
#include "err.hpp"

static const int gsize = 128;
static const int defaultfd = -1;

class Epoller
{
public:
    Epoller():epfd_(defaultfd)
    {}

    void Create()
    {
        epfd_ = epoll_create(gsize);
        if(epfd_ < 0)
        {
            logMessage(Fatal,"Epoll create fail,code:%d,errstring:%s",errno,strerror(errno));
            exit(EPOLL_CREATE_ERR);
        }
    }

    bool AddModEvent(int fd,uint32_t events,int op)
    {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = fd; 
        int n = epoll_ctl(epfd_,op,fd,&ev);
        if(n < 0)
        {
            logMessage(Fatal,"epoll_ctl error,code:%d,errstring:%s",errno,strerror(errno));
            return false;
        }
        return true;
    }

    int Wait(struct epoll_event* revs,int num,int timeout)
    {
        return epoll_wait(epfd_,revs,num,timeout);
    }
    bool DelEvent(int fd)
    {
        return epoll_ctl(epfd_,EPOLL_CTL_DEL,fd,nullptr) == 0;
    }

    int Fd()
    {
        return epfd_;
    }

    void Close()
    {
        if(epfd_ != defaultfd) close(epfd_);
    }

    ~Epoller()
    {}

private:
    int epfd_;
};