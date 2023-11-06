#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "log.hpp"
#include <string.h>
#include "err.hpp"

class sock
{
    static const int defaultfd = -1;
    static const int backlog = 32;
public:
    sock():_sock(defaultfd)
    {}

    void Socket()
    {
        _sock = socket(AF_INET,SOCK_STREAM,0);
        if(_sock<0)
        {
            logMessage(Fatal,"create socket error,code:%d,errorstring:%s",errno,strerror(errno));
            exit(SOCKET_ERR);
        }
        logMessage(Debug,"create socket success,code:%d,errorstring:%s",errno,strerror(errno));
    }

    void Bind(const uint16_t port)
    {
        struct sockaddr_in local;
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        local.sin_port = htons(port);
        int n = bind(_sock,(struct sockaddr*)&local,sizeof(local));
        if(n < 0)
        {
            logMessage(Fatal,"binding tcpserver failed,errno:%d,errorstring:%s",errno,strerror(errno));
            exit(BIND_ERR);
        }
        logMessage(Debug,"binding tcpserver success,errno:%d,errorstring:%s",errno,strerror(errno));
    }

    void Listen()
    {
        if(listen(_sock,backlog))
        {
            logMessage(Fatal,"listen failed,errno:%d,stringerror:%s",errno,strerror(errno));
            exit(LISTEN_ERR);
        }
        logMessage(Debug,"listen success,errno:%d,stringerror:%s",errno,strerror(errno));
    }

    int Accept(std::string *clientip,uint16_t* clientport)
    {
        struct sockaddr_in temp;
        socklen_t len;
        int sock = accept(_sock,(struct sockaddr*)&temp,&len);
        if(sock < 0)
        {
            logMessage(Warnning,"accept error,errno:%d,stringerror:%s",errno,strerror(errno));
        }
        else
        {
            *clientip = inet_ntoa(temp.sin_addr);
            *clientport = ntohs(temp.sin_port);
        }

        return sock;
    }

    int Fd()
    {
        return _sock;
    }

    int Connect(const std::string serverip,const uint16_t serverport)
    {
        struct sockaddr_in local;
        bzero(&local,sizeof(local));
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = inet_addr(serverip.c_str());
        local.sin_port = htons(serverport);

        return connect(_sock,(struct sockaddr*)&local,sizeof(local));
    }

    void Close()
    {
        if(_sock != defaultfd)
            close(_sock);
    }

    ~sock()
    {
        
    }
private:
    int _sock;
};
