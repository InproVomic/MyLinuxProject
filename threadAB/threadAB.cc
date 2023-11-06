#include <pthread.h>
#include <iostream>
#include <unistd.h>

pthread_mutex_t mutex;

int cnt = 100;
void* func(void* args)
{
    while(cnt)
    {
        pthread_mutex_lock(&mutex);
        std::cout<<"我是线程"<<(char*)args<<std::endl;
        --cnt;
        pthread_mutex_unlock(&mutex);
        usleep(2000);
    }

    return nullptr;
}

int main()
{
    pthread_t threadA;
    pthread_t threadB;

    pthread_create(&threadA,nullptr,func,(char*)"A");
    pthread_create(&threadB,nullptr,func,(char*)"B");

    pthread_join(threadA,nullptr);
    pthread_join(threadB,nullptr);
    return 0;
}