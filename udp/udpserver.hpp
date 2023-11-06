#pragma once
#include "Thread.hpp"
#include "RingQueue.hpp"
#include "LockGurad.hpp"
#include "err.hpp"
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <map>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <vector>

const static uint16_t default_port = 8080;

class udpserver
{
public:
  udpserver(const uint16_t port = default_port) : _port(port)
  {
    pthread_mutex_init(&_lock, nullptr);
    _p = new Thread(1, std::bind(&udpserver::Recv, this));
    _c = new Thread(1, std::bind(&udpserver::Broadcast, this));
  }

  void star()
  {
    _sock = socket(AF_INET, SOCK_DGRAM, 0);//创建套接字
    if (_sock < 0)
    {
      std::cerr << "create socket error" << strerror(errno) << std::endl;
      exit(SOCKET_ERR);
    }

    struct sockaddr_in lock;
    bzero(&lock, sizeof(lock));
    lock.sin_family = AF_INET;
    lock.sin_addr.s_addr = INADDR_ANY;
    lock.sin_port = htons(_port);//将主机数据转化成网络数据（其实就是转化成大端数据）

    if (bind(_sock, (struct sockaddr *)&lock, sizeof(lock)) < 0)
    {
      std::cerr << "bind error" << strerror(errno) << std::endl;
      exit(BIND_ERR);
    }

    //让线程跑起来！
    _p->run();
    _c->run();
  }

  void addUser(std::string& name,const struct sockaddr_in& peer)
  {
    LockGuard Lock(&_lock);
    auto iter = _onlineuser.find(name);//这里还要检查是为了提高效率
    if(iter != _onlineuser.end())
      return;

    _onlineuser.insert(std::make_pair(name,peer));
  }

  void Recv()
  {
    char buffer[1024];//创建接收缓冲区
    while(true)
    {
      struct sockaddr_in local;
      socklen_t len = sizeof(local);
      int n = recvfrom(_sock,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&local,&len);//接取收到的数据信息 
      
      
      if(n > 0)
        buffer[n] = '\0';//在缓冲区最后一位弄成'\0'
      else 
        continue;

      std::cout<<"recv done..."<<std::endl;

      std::string clientip = inet_ntoa(local.sin_addr);//要将网络数据转化为主机数据
      uint16_t clientport = ntohs(local.sin_port);//同上
      std::cout<<clientip<<"-"<<clientport<<"#"<<buffer<<std::endl;

      std::string name = clientip;
      name += "-";
      name += std::to_string(clientport);

      addUser(name,local);//往_onlineuser添加用户
      _rq.push(buffer);//把要处理的数据放进生产消费者模型的环形队列中
    }
  }

  void Broadcast()
  {
    while(true)
    {
      std::string sendstring;
      _rq.pop(&sendstring);//取出已经处理完的数据

      std::vector<struct sockaddr_in> v;//用来保存用户的
      {
        LockGuard lockguard(&_lock);//这里要加锁，不然可能造成map添加元素出问题
        for(auto& e : _onlineuser)
        {
          v.push_back(e.second);//将每个用户放到vector中，方便遍历
        }
      }

      for(auto& user : v)
      {
        sendto(_sock,sendstring.c_str(),sizeof(sendstring),0,(struct sockaddr*)&user,sizeof(user));//向每个用户发送数据（其实就是群组聊天的功能）
        std::cout<<"send done..."<<sendstring<<std::endl;
      }
      
    }
  }

  ~udpserver()
  {
    pthread_mutex_destroy(&_lock);
    _p->join();
    _c->join();

    delete _c;
    delete _p;
  }

private:
  int _sock;                  // sock套接字
  uint16_t _port;             // 服务端端口号
  RingQueue<std::string> _rq; // 生产消费者模型环形队列
  pthread_mutex_t _lock;      // 锁
  std::map<std::string, struct sockaddr_in> _onlineuser;
  Thread *_c; // 消费者线程
  Thread *_p; // 生产者线程
};
