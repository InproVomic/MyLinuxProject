#include <iostream>
#include <signal.h>
#include <unistd.h>

void sigcb(int signum)
{
    std::cout<<"本次事件的信号值为："<<signum<<std::endl;
}

int main()
{
    struct sigaction new_action,old_action;
    new_action.sa_flags = 0;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_handler = sigcb;

    sigaction(SIGINT,&new_action,&old_action);

    while(true)
    {
        std::cout<<"我还活得好好的！"<<getpid()<<std::endl;
        sleep(2);
    }

    return 0;
}