#pragma once
#include <iostream>
#include <pthread.h>
#include <string>
#include "sock.hpp"
#include <functional>

static const uint16_t port = 8888;
using func_t = std::function<std::string(std::string& )>;


class HttpServer;
class ThreadData
{
public:
    ThreadData(const std::string ip,const uint16_t port,const int sock,HttpServer* tsvrp)
    :_ip(ip),_port(port),_sock(sock),_tsvrp(tsvrp)
    {}
    ~ThreadData()
    {}

public:
    int _sock;  
    std::string _ip;
    uint16_t _port;
    HttpServer* _tsvrp;
};

class HttpServer
{
public:
    HttpServer(const uint16_t port,func_t func):_port(port),_func(func)
    {}

    void InitServer()
    {
        _listensock.Socket();
        _listensock.Bind(_port);
        _listensock.Listen();
    }

    void HandlerHttpRequest(int sock)
    {
        char buffer[4096];
        std::string request;
        ssize_t s = recv(sock,buffer,sizeof(buffer)-1,0);
        if(s > 0)
        {
            buffer[s] = 0;
            request = buffer;
            std::string respond = _func(request);
            send(sock,respond.c_str(),respond.size(),0);
        }
        else
        {
            logMessage(Info,"Clinet quit...");
        }
    }

    static void* threadRoutine(void* args)
    {
        pthread_detach(pthread_self());
        ThreadData* td = static_cast<ThreadData*>(args);
        td->_tsvrp->HandlerHttpRequest(td->_sock);
        close(td->_sock);
        delete td;
        return nullptr;
    }

    void start()
    {
        while(true)
        {
            std::string clientip;
            uint16_t clientport;
            int sock = _listensock.Accept(&clientip,&clientport);
            if(sock < 0) continue;

            ThreadData* td = new ThreadData(clientip,clientport,sock,this);
            pthread_t tid;
            pthread_create(&tid,nullptr,threadRoutine,td);
        }
    }

    ~HttpServer()
    {}
private:
    sock _listensock;
    uint16_t _port;
    func_t _func;
};