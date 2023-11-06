#include "threadPool.hpp"
#include "Task.hpp"
#include <memory>

int o = 0;

int main()
{
    std::string opers = "+-*/%";
    while (true)
    {
        int x = rand() % 20 + 1;
        int y = rand() % 10 + 1;
        char op = opers[rand() % opers.size()];

        std::cout<<"x = "<<x<<" and y = "<<y<<"with operate is-("<<op<<")"<<std::endl;
        Task t(x, y, op);
        threadPool<Task>::getinstance()->pushTask(t); //单例对象也有可能在多线程场景中使用！
        usleep(500000);
        
        // tp->pushTask(t);

        // 充当生产者, 从网络中读取数据，构建成为任务，推送给线程池
        // sleep(1);
        // tp->pushTask();
    }

    return 0;
}