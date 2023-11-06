#pragma once
#include "threadPool.hpp"
#include "sock.hpp"
#include "Thread.hpp"
#include <functional>
#include "Protocol.hpp"

const static uint16_t defaultport = 8081;
static const int backlog = 32;

class tcpserver;

class ThreadData
{
public:
    ThreadData(const std::string clientip, const uint16_t clientport, int sock, tcpserver *current) : _sock(sock), _clientip(clientip), _clinetport(clientport), _current(current)
    {
    }

    int _sock;
    std::string _clientip;
    uint16_t _clinetport;
    tcpserver *_current;
};

class tcpserver
{
public:
    using func_t = std::function<Response(const Request &)>;
    tcpserver(func_t func, const uint16_t port) : _port(port), _func(func)
    {
    }

    void InitServer()
    {
        _listensock.Socket();
        _listensock.Bind(_port);
        _listensock.Listen();
        logMessage(Info, "init server done...,listensock:%d", _listensock.Fd());
    }

    void Start()
    {
        while (true)
        {
            std::string clientip;
            uint16_t clientport;
            int sock = _listensock.Accept(&clientip, &clientport);

            if (sock < 0)
                continue;
            
            logMessage(Debug,"get a new client,client info:[%s,%d]",clientip.c_str(),clientport);
            pthread_t tid;
            ThreadData* td = new ThreadData(clientip,clientport,sock,this);
            pthread_create(&tid,nullptr,ThreadRoutine,td);
        }
    }

    static void* ThreadRoutine(void* args)
    {
        pthread_detach(pthread_self());
        ThreadData* td = static_cast<ThreadData* >(args);
        td->_current->ServiceIO(td->_sock,td->_clientip,td->_clinetport);
        logMessage(Debug,"thread quit,client quit,...");
        delete td;
        return nullptr;
    }

    void ServiceIO(int sock,const std::string& ip,const uint16_t& port)
    {  
        std::string inbuffer;
        while(true)
        {
            std::string package;
            int n = ReadPackage(sock,inbuffer,&package);
            if(n == -1)
                break;
            else if(n == 0)
                continue;
            else
            {
                package = RemoveHeader(package,n);

                Request res;

                res.Deserialize(package);

                Response rep = _func(res);//业务处理

                std::string send_string;
                rep.Serialize(&send_string);

                send_string = AddHeader(send_string);

                send(sock,send_string.c_str(),send_string.size(),0);
            }
        }
        close(sock);
    }

    ~tcpserver()
    {
        _listensock.Close();
    }

private:
    sock _listensock;
    uint16_t _port;
    func_t _func;
};