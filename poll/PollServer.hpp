#pragma once

#include <iostream>
#include <sys/poll.h>
#include "sock.hpp"
#include "log.hpp"
#include <map>

const static uint16_t gport = 8888;
const static short defaultevent = 0;
const int N = 4096;
const static int defaultfd = -1;
typedef struct pollfd type_t;

class PollServer
{
public:
    PollServer(const uint16_t port = gport):port_(port)
    {}

    void InitServer()
    {
        listensock_.Socket();
        listensock_.Bind(port_);
        listensock_.Listen();

        fdarry_ = new type_t[N];
        for(int i = 0;i<N;++i)
        {
            fdarry_[i].events = defaultevent;
            fdarry_[i].fd = defaultfd;
            fdarry_[i].revents = defaultevent;
        }

        wordmap.insert(std::make_pair("apple","苹果\n"));
        wordmap.insert(std::make_pair("english","英语\n"));
        wordmap.insert(std::make_pair("liella","星团\n"));
        wordmap.insert(std::make_pair("futher","未来\n"));
        wordmap.insert(std::make_pair("dream","梦想\n"));
    }

    void Accepter()
    {
        uint16_t clientport;
        std::string clientip;
        int sock = listensock_.Accept(&clientip,&clientport);

        if(sock < 0)
            return;

        int pos = 1;
        for(;pos < N;++pos)
        {
            if(fdarry_[pos].fd == defaultfd)
                break;
        }
        if(pos >= N)
        {
            close(sock);
            logMessage(Warnning,"fdarry full, %d: %s",errno,strerror(errno));
        }
        else
        {
            fdarry_[pos].events = POLLIN;
            fdarry_[pos].fd = sock;
            fdarry_[pos].revents = defaultevent;
        }
    }

    void Recver(int index)
    {
        int fd = fdarry_[index].fd;
        char buffer[1024];
        ssize_t s = recv(fd,buffer,sizeof(buffer)-1,0);
        if(s > 0)
        {
            buffer[s-2] = 0;
            std::string echo;
            echo += buffer;
            auto it = wordmap.find(buffer);
            if(it == wordmap.end())
                echo += "Can't find the word.\n";
            else
                echo += it->second;
            
            ssize_t n = send(fd,echo.c_str(),echo.size(),0);
        }
        else
        {
            if(s == 0)
                logMessage(Info,"Client quit....,fdarry[%d],fd->%d",index,fd);
            else
                logMessage(Warnning,"recv error,fdarry[%d],fd->%d",index,fd);

            close(fd);
            fdarry_[index].fd = defaultfd;
            fdarry_[index].events = defaultevent;
            fdarry_[index].revents = defaultevent;
        }
    }

    void HandlerEvent()
    {
        for(int i = 0;i < N;++i)
        {
            int fd = fdarry_[i].fd;
            short revent = fdarry_[i].revents;
            if(fdarry_[i].fd == defaultfd)
                continue;
            
            if(fd == listensock_.Fd() && (revent & POLLIN))
            {
                Accepter();
            }
            else if(fd != listensock_.Fd() && (revent & POLLIN))
            {
                Recver(i);
            }
            else
            {}
        }
    }

    void Start()
    {
        fdarry_[0].fd = listensock_.Fd();
        fdarry_[0].events = POLLIN;
        while(true)
        {
            int timeout = -1;
            int n = poll(fdarry_,N,timeout);
            
            switch (n)
            {
            case 0:
                logMessage(Info,"time out,%d: %s",errno,strerror(errno));
                break;
            case -1:
                logMessage(Warnning,"%d: %s",errno,strerror(errno));
            default:
                logMessage(Debug,"有一个就绪时间发生了,%d: %s",errno,strerror(errno));
                HandlerEvent();
                DebugPrint();
                break;
            }
        }
    }

    void DebugPrint()
    {
        std::cout << "fdarray[]: ";
        for (int i = 0; i < N; i++)
        {
            if (fdarry_[i].fd == defaultfd)
                continue;
            std::cout << fdarry_[i].fd << " ";
        }
        std::cout << "\n";
    }

    ~PollServer()
    {
        listensock_.Close();
    }

private:
    sock listensock_;
    type_t* fdarry_;
    uint16_t port_;
    std::map<std::string,std::string> wordmap; 
};

