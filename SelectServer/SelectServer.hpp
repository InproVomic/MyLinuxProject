#pragma once

#include <iostream>
#include <sys/select.h>
#include "sock.hpp"
#include "log.hpp"
#include <map>

const static uint16_t gport = 8888;
const int defaultfd = -1;
const uint8_t defaultevent = 0;
const int N = sizeof(fd_set)*8;

#define READ_EVENT (0x1)
#define WRITE_EVENT (0X1<<1)
#define EXCEPT_EVENT (0X<<2)

typedef struct FdEvent
{
    uint16_t clientport;
    std::string clientip;
    int fd;
    uint8_t event;
}type_t;

class SelectServer
{
public:
    SelectServer(const uint16_t port = gport):port_(port)
    {}

    void InitServer()
    {
        listensock_.Socket();
        listensock_.Bind(port_);
        listensock_.Listen();
        for(int i = 0;i<N;++i)
        {
            fdarry_[i].event = defaultevent;
            fdarry_[i].fd = defaultfd;
            fdarry_[i].clientport = 0;
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
            fdarry_[pos].clientip = clientip;
            fdarry_[pos].clientport = clientport;
            fdarry_[pos].event = READ_EVENT;
            fdarry_[pos].fd = sock;
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

            close(fdarry_[index].fd);
            fdarry_[index].clientip.resize(0);
            fdarry_[index].clientport = 0;
            fdarry_[index].fd = defaultfd;
            fdarry_[index].event = defaultevent;
        }
    }

    void HandlerEvent(fd_set& rfds,fd_set& wfds)
    {
        for(int i = 0;i < N;++i)
        {
            if(fdarry_[i].fd == defaultfd)
                continue;
            if(fdarry_[i].event & READ_EVENT && FD_ISSET(fdarry_[i].fd,&rfds))
            {
                if(fdarry_[i].fd == listensock_.Fd())
                    Accepter();
                else if(fdarry_[i].fd != listensock_.Fd())
                    Recver(i);
                else
                {}
            }
            else if(fdarry_[i].event & WRITE_EVENT && FD_ISSET(fdarry_[i].fd,&wfds))
            {
                //暂时没需求
            }
            else
            {
                //暂时没需求
            }
        }
    }

    void Start()
    {
        fdarry_[0].fd = listensock_.Fd();
        fdarry_[0].event = READ_EVENT;
        while(true)
        {
            fd_set rfds;
            fd_set wfds;
            FD_ZERO(&rfds);
            FD_ZERO(&wfds);   
            int maxfd = fdarry_[0].fd;
            for(int i = 0;i < N;++i)
            {
                if(fdarry_[i].fd == defaultfd)
                    continue;
                if(fdarry_[i].event & READ_EVENT)
                    FD_SET(fdarry_[i].fd,&rfds);
                if(fdarry_[i].event & WRITE_EVENT)
                    FD_SET(fdarry_[i].fd,&wfds);
                if(maxfd < fdarry_[i].fd)
                    maxfd = fdarry_[i].fd;
            }
            int n = select(maxfd +1,&rfds,&wfds,nullptr,nullptr);
            
            switch (n)
            {
            case 0:
                logMessage(Info,"time out,%d: %s",errno,strerror(errno));
                break;
            case -1:
                logMessage(Warnning,"%d: %s",errno,strerror(errno));
            default:
                logMessage(Debug,"有一个就绪时间发生了,%d: %s",errno,strerror(errno));
                HandlerEvent(rfds,wfds);
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

    ~SelectServer()
    {
        listensock_.Close();
    }

private:
    sock listensock_;
    type_t fdarry_[N];
    uint16_t port_;
    std::map<std::string,std::string> wordmap; 
};

