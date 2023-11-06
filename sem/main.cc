#include "RingQueue.hpp"
#include "Task.hpp"

void* produce(void* args)
{
    RingQueue<Task>* bq = static_cast<RingQueue<Task>*>(args);
    std::string opers = "+-*/%";
    while(true)
    {
        int x = rand() % 20 + 1;
        int y = rand() % 10 + 1;
        char op = opers[rand() % opers.size()];

        Task t(x,y,op);
        bq->push(t);
        std::cout << pthread_self() << " | productor Task: " <<  t.formatArg() << "?" << std::endl;
    }

    return nullptr;
}

void* consume(void* args)
{
    RingQueue<Task>* bq = static_cast<RingQueue<Task>*>(args);
    while(true)
    {

        Task t;
        bq->pop(&t);
        std::cout<<"1"<<std::endl;
        t();

        std::cout << pthread_self() << " | consumer data: " << t.formatArg() << t.formatRes() << std::endl;
    }

    return nullptr;
}

int main()
{
    RingQueue<Task>* bq = new RingQueue<Task>();
    pthread_t consumers[3],producers[3];
    for(int i = 0;i<3;++i)
    {
        pthread_create(&producers[i],nullptr,produce,bq);
    }
    for(int i = 0;i<3;++i)
    {
        pthread_create(&consumers[i],nullptr,consume,bq);
    }

    pthread_join(consumers[0],nullptr);
    pthread_join(consumers[1],nullptr);
    pthread_join(consumers[2],nullptr);
    pthread_join(producers[0],nullptr);
    pthread_join(producers[1],nullptr);
    pthread_join(producers[2],nullptr);

    delete bq;
    return 0;
}