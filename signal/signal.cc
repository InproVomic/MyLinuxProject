#include <iostream>
#include <signal.h>
#include <unistd.h>

void sigcb(int signum)
{
    std::cout<<"本次事件的信号值为："<<signum<<std::endl;
}

int main()
{
    signal(SIGINT,sigcb);
    while(true)
    {
        sleep(2);
        std::cout<<"我是待杀进程"<<std::endl;
    }
    return 0;
}