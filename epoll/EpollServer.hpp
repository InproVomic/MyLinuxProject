#include "Util.hpp"
#include "Protocol.hpp"
#include "Epoller.hpp"
#include <unordered_map>
#include "sock.hpp"
#include <functional>

class Connection;
class EpollServer;

const static int gport = 8880;
const static int bsize = 1024;
using func_t = std::function<Response(const Request &)>;
using callback_t = std::function<void(Connection *)>;

class Connection
{
public:
    Connection(const int &fd, const std::string &clientip, const uint16_t clientport)
        : fd_(fd), clientip_(clientip), clientport_(clientport)
    {
    }

    void Register(callback_t recver, callback_t sender, callback_t excepter)
    {
        recver_ = recver;
        sender_ = sender;
        excepter_ = excepter;
    }

    ~Connection()
    {
    }

public:
    // IO信息
    int fd_;
    std::string inbuff_;
    std::string outbuff_;
    // IO函数
    callback_t recver_;
    callback_t sender_;
    callback_t excepter_;

    // 用户信息
    std::string clientip_;
    uint16_t clientport_;

    uint32_t events;

    // 回指指针
    EpollServer *R;

    // 时间戳，最近一次就绪的时间
    time_t lasttime;
};

class EpollServer
{
    static const int gnum = 64;

public:
    EpollServer(func_t func, uint16_t port = gport) : func_(func), port_(port)
    {
    }

    void InitServer()
    {
        listensock_.Socket();
        listensock_.Bind(port_);
        listensock_.Listen();
        epoller_.Create();
        AddConnection(listensock_.Fd(), EPOLLIN | EPOLLET);
    }

    void Dispatcher()//事件派发器
    {
        int timeout = 1000;
        while(true)
        {
            LoopOnce(timeout);
        }
    }

    void LoopOnce(int timeout)
    {
        int n = epoller_.Wait(revs_,gnum,timeout);
        for(int i = 0;i < gnum;++i)
        {
            int fd = revs_[i].data.fd;
            uint32_t events = revs_[i].events;
            logMessage(Debug,"当前正在处理%d上的%s",fd,(events & EPOLLIN)?"EPOLLIN":"OTHER");

            if((events & EPOLLERR) || (events & EPOLLHUP))
                events |= (EPOLLIN | EPOLLOUT);

            if((events & EPOLLIN) && ConnIsExists(fd))
                Connections_[fd]->recver_(Connections_[fd]);

            if((events & EPOLLOUT) && ConnIsExists(fd))
                Connections_[fd]->sender_(Connections_[fd]);
        }
    }

    void AddConnection(int fd, uint32_t events, std::string ip = "127.0.0.1", uint16_t port = gport)
    {
        if (events & EPOLLET)
            Util::SetNonBlock(fd);

        Connection *conn = new Connection(fd, ip, port);
        if (fd == listensock_.Fd())
        {
            conn->Register(std::bind(&EpollServer::Accepter, this, std::placeholders::_1), nullptr, nullptr);
        }
        else
        {
            conn->Register(std::bind(&EpollServer::Recver, this, std::placeholders::_1),
                           std::bind(&EpollServer::Sender, this, std::placeholders::_1),
                           std::bind(&EpollServer::Excepter, this, std::placeholders::_1));
        }
        conn->events = events;
        conn->fd_ = fd;
        conn->lasttime = time(nullptr);
        conn->R = this;
        Connections_.insert(std::pair<int, Connection *>(fd, conn));
        bool r = epoller_.AddModEvent(fd, events, EPOLL_CTL_ADD);
        (void)r;
    }

    void Accepter(Connection *conn)
    {
        do
        {
            int err = 0;
            std::string clientip;
            uint16_t clientport;
            int sock = listensock_.Accept(&clientip, &clientport,&err);
            if (sock > 0)
            {
                logMessage(Debug, "%s:%d,已连接上服务器",clientip.c_str(),clientport);
                AddConnection(sock,EPOLLIN | EPOLLET,clientip,clientport);
            }
            else
            {
                if(err == EAGAIN || err == EWOULDBLOCK)
                    break;
                else if(err == EINTR)
                    continue;
                else
                {
                    logMessage(Warnning,"errstring:%s,errcode:%d",strerror(err),err);
                    break;
                }
            }
        } while (conn->events & EPOLLET);
        logMessage(Debug,"Accepter done...");
    }

    void Recver(Connection *conn)
    {
        if(!RecverHelper(conn))return;
        HandlerRequest(conn);

        if(!conn->outbuff_.empty())
            Sender(conn);
    }

    void Sender(Connection *conn)
    {
        bool safe = true;
        do
        {
            int n = send(conn->fd_,conn->outbuff_.c_str(),conn->outbuff_.size(),0);
            if(n > 0)
            {
                conn->outbuff_.erase(0,n);
                if(conn->outbuff_.empty())
                    break;
            }
            else
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                else if(errno == EINTR)
                    continue;
                else
                {
                    safe = false;
                    conn->excepter_(conn);
                    break;
                }
            }
        } while(conn->events & EPOLLET);
        
        if(!safe)return;
        
        if(!conn->outbuff_.empty())
            EnableReadWrite(conn,true,true);
        else
            EnableReadWrite(conn,true,false);
    }

    void Excepter(Connection *conn)
    {
        epoller_.DelEvent(conn->fd_);//移除epoll中的fd

        close(conn->fd_);//关闭连接
        
        Connections_.erase(conn->fd_);//移除connections_中的映射

        delete conn;//删除对象
    }

    bool RecverHelper(Connection* conn)
    {
        bool ret = true;
        conn->lasttime = time(nullptr);
        do
        {
            char buff[bsize];
            ssize_t s = recv(conn->fd_,buff,sizeof(buff)-1,0);
            if(s > 0)
            {
                buff[s] = 0;
                conn->inbuff_ += buff;
            }
            else if(s == 0)
            {
                conn->excepter_(conn);
                ret = false;
                break;
            }
            else
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                else if(errno == EINTR)
                    continue;
                else
                {
                    conn->excepter_(conn);
                    ret = false;

                    break;
                }
            }

        }while(conn->events & EPOLLET);

        return ret;
    }

    void HandlerRequest(Connection* conn)
    {
        int quit = false;
        while(!quit)
        {
            std::string package;
            int n = ParsePackage(conn->inbuff_,&package);//1.提取报文和有效载荷长度
            if(n > 0)
            {
                package = RemoveHeader(package,n);//2.提取有效载荷

                Request req;
                req.Deserialize(package);//3.反序列化

                Response resp = func_(req);//4.处理请求
                
                std::string RespStr;
                resp.Serialize(&RespStr);//5.序列化

                RespStr = AddHeader(RespStr);//6.添加报头

                conn->outbuff_ += RespStr;//7.将要发送的数据放入发送缓冲区中
            }
            else
                quit = true;
        }
    }

    bool ConnIsExists(int fd)
    {
        return Connections_.find(fd) != Connections_.end();
    }

    bool EnableReadWrite(Connection* conn,bool readable,bool writeable)
    {
        conn->events = ((readable ? EPOLLIN : 0)| (writeable ? EPOLLOUT : 0)|EPOLLET);
        return epoller_.AddModEvent(conn->fd_,conn->events,EPOLL_CTL_MOD);
    }

    ~EpollServer()
    {
        listensock_.Close();
        epoller_.Close();
    }

private:
    uint16_t port_;
    sock listensock_;
    Epoller epoller_;
    struct epoll_event revs_[gnum];
    func_t func_;
    std::unordered_map<int, Connection *> Connections_;
};