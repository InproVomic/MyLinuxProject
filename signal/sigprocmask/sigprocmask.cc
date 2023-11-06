#include <iostream>
#include <signal.h>
#include <unistd.h>

int main()
{
    sigset_t set,oldset;
    sigemptyset(&set);
    sigemptyset(&oldset);

    sigaddset(&set,2);
    sigaddset(&set,40);

    sigprocmask(SIG_BLOCK,&set,&oldset);

    int cnt = 5;

    while(cnt)
    {
        kill(getpid(),2);
        kill(getpid(),40);
        std::cout<<"已经发送"<<cnt<<"次2号信号"<<std::endl;
        std::cout<<"已经发送"<<cnt--<<"次40号信号"<<std::endl;
        sleep(2);
    }

    return 0;
}