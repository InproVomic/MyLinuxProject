#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <string>

int cnt = 0;
pthread_mutex_t mutex;

void* CountNum(void* args)
{
    while(true)
    {
        if(cnt<100)
        {
            pthread_mutex_lock(&mutex);
            std::string name = static_cast<const char *>(args);
            ++cnt;
            std::cout<<name<<" : "<<cnt<<std::endl;
            pthread_mutex_unlock(&mutex);
            usleep(2000);
        }
        else
        {
            pthread_mutex_unlock(&mutex);
            break;
        }
    }

    return nullptr;
}

int main()
{
    pthread_t Odd,Even;
    pthread_mutex_init(&mutex,nullptr);
    pthread_create(&Odd,nullptr,CountNum,(char*)"线程1");
    pthread_create(&Even,nullptr,CountNum,(char*)"线程2");
    
    pthread_join(Odd,nullptr);
    pthread_join(Even,nullptr);
    return 0;
}