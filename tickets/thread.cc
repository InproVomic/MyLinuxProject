#include <iostream>
#include <pthread.h>
#include <string>
#include <unistd.h>

int tickets = 10000;
pthread_mutex_t mutex;

void *func1(void *args)
{
    std::string name = static_cast<const char *>(args);
    while (true)
    {
        pthread_mutex_lock(&mutex);
        if (tickets > 0)//这里的if很重要，如果是改成while(tickets > 0)，就算加了mutex也会出现有抢票抢到负数的情况！
        {
            usleep(2000);
            --tickets;
            std::cout << "我是" << name << "当前剩余票数为" << tickets << "张" << std::endl;
            pthread_mutex_unlock(&mutex);
        }
        else
        {
            pthread_mutex_unlock(&mutex);
            break;
        }
        usleep(1000);
    }
    delete[] static_cast<const char *>(args);
    return nullptr;
}

int main()
{
    pthread_mutex_init(&mutex, nullptr);
    pthread_t newthread[5];
    for (int i = 0; i < 5; ++i)
    {
        char *newchar = new char[64];
        snprintf(newchar, 64, "thread-%d", i+1);
        pthread_create(&newthread[i], nullptr, func1, newchar);
    }

    for (int i = 0; i < 5; ++i)
    {
        pthread_join(newthread[i], nullptr);
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}