#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <string.h>
#include <arpa/inet.h>

static void usage(std::string proc)
{
    std::cout<<"Usage:\n\t"<<proc<<" serverip serverport\n"<<std::endl;
}

int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        usage(argv[0]);
    }

    std::string serverip = argv[1];
    uint16_t serverport = atoi(argv[2]);
    int _sock = socket(AF_INET,SOCK_DGRAM,0);
    if(_sock < 0)
    {
        std::cerr<<"create socket error"<<strerror(errno)<<std::endl;
    }

    sockaddr_in local;
    bzero(&local,sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(serverport);
    local.sin_addr.s_addr = inet_addr(serverip.c_str());

    while(true)
    {
        std::cout<<"[myserver] #";
        std::string messages;
        std::getline(std::cin,messages);

        sendto(_sock,messages.c_str(),messages.size(),0,(struct sockaddr*)&local,sizeof(local));

        char buffer[2048];
        sockaddr_in temp;
        socklen_t len;
        int n = recvfrom(_sock,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&temp,&len);
        if(n > 0)
        {
            buffer[n] = '\0';
            std::cout<<buffer<<std::endl;
        }
    }

    return 0;
}
