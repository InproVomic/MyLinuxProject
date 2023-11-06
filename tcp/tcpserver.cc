#include "tcpserver.hpp"
#include <memory>

Response calculate(const Request& res)
{
    Response rep(0,0);
    switch (res._op)
    {
    case '+':
        rep._result = res._x + res._y;
        break;
    case '-':
        rep._result = res._x - res._y;
        break;
    case '*':
        rep._result = res._x * res._y;
        break;
    case '/':
        if(res._y == 0)
            rep._code = 1;
        else
            rep._result = res._x / res._y;
        break;
    case '%':
        if(res._y == 0)
            rep._code = 2;

        else
            rep._result = res._x % res._y;
        break;
    default:
        rep._code = 3;
        break;
    }

    return rep;
}

int main()
{
    uint16_t port = 8081;
    std::unique_ptr<tcpserver> tsvr(new tcpserver(calculate,port));
    tsvr->InitServer();
    tsvr->Start();
    
    return 0;
}