#include "udpserver.hpp"
#include <memory>

static void usage(std::string proc)
{
    std::cout<<"Usage:\n\t"<<proc<<" port\n"<<std::endl;
}

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port = atoi(argv[1]);
    std::unique_ptr<udpserver> server(new udpserver(port));

    server->star();

    return 0;
}
