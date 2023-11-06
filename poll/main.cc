#include "PollServer.hpp"
#include <memory>

int main()
{
    std::unique_ptr<PollServer> svr(new PollServer);
    svr->InitServer();
    svr->Start();


    return 0;
}