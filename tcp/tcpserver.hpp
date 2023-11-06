#pragma once
#include "threadPool.hpp"
#include "sock.hpp"
#include "Thread.hpp"
#include <functional>
#include "Protocol.hpp"
#include <sys/wait.h>
#include <signal.h>

const static uint16_t defaultport = 8081;
static const int backlog = 32;

class tcpserver;

// class ThreadData
// {
// public:
//     ThreadData(const std::string clientip, const uint16_t clientport, int sock, tcpserver *current) : _sock(sock), _clientip(clientip), _clinetport(clientport), _current(current)
//     {
//     }

//     int _sock;
//     std::string _clientip;
//     uint16_t _clinetport;
//     tcpserver *_current;
// };

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
        signal(SIGCHLD,SIG_IGN);//让子进程退出时发送的信号忽略，让子进程自己回收资源
        while (true)
        {
            std::string clientip;
            uint16_t clientport;
            int sock = _listensock.Accept(&clientip, &clientport);

            if (sock < 0)
                continue;

            logMessage(Debug, "get a new client,client info:[%s,%d]", clientip.c_str(), clientport);
            // V1多线程版本
            //  pthread_t tid;
            //  ThreadData* td = new ThreadData(clientip,clientport,sock,this);
            //  pthread_create(&tid,nullptr,ThreadRoutine,td);

            // V2多进程版本
            pid_t id = fork();
            if (id < 0)
            {
                close(sock);
                continue;
            }
            else if (id == 0) // child
            {
                close(_listensock.Fd()); // 关闭不需要的fd
                // 关掉子进程，保留孙子进程，孙子进程是守护进程！
                // if (fork() > 0)
                //     exit(0);
                this->ServiceIO(sock, clientip, clientport);
                logMessage(Debug, "thread quit,client quit,...");
                exit(0);
            }
            else
            {
                close(sock);
                pid_t ret = waitpid(id, nullptr, 0);
                if (ret == id)
                    std::cout << "wait child " << id << " success" << std::endl;
            }
        }
    }

    // static void* ThreadRoutine(void* args)
    // {
    //     pthread_detach(pthread_self());
    //     ThreadData* td = static_cast<ThreadData* >(args);
    //     td->_current->ServiceIO(td->_sock,td->_clientip,td->_clinetport);
    //     logMessage(Debug,"thread quit,client quit,...");
    //     delete td;
    //     return nullptr;
    // }

    void ServiceIO(int sock, const std::string &ip, const uint16_t &port)
    {
        std::string inbuffer;
        while (true)
        {
            std::string package;
            int n = ReadPackage(sock, inbuffer, &package);
            logMessage(Debug, "ReadPackage return n:%d", n);
            if (n == -1)
                break;
            else if (n == 0)
            {
                logMessage(Debug, "log in else if,and n:%d",n);
                continue;
            }
            else
            {
                package = RemoveHeader(package, n);

                Request res;

                res.Deserialize(package);

                Response rep = _func(res); // 业务处理

                std::string send_string;
                rep.Serialize(&send_string);

                send_string = AddHeader(send_string);
                logMessage(Debug, "had AddHeader");

                send(sock, send_string.c_str(), send_string.size(), 0);
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