#include "sock.hpp"
#include "Protocol.hpp"
#include <string>

static void usage(std::string proc)
{
    std::cout<<"Usage:\r\n"<<proc<<" serverip serverport"<<std::endl;
}

enum
{
    LEFT,
    OPER,
    RIGHT
};

Request ParseLine(const std::string& line)
{
    std::string left,right;
    char op;
    int status = LEFT;
    int i = 0;
    while(i < line.size())
    {
        switch (status)
        {
        case LEFT:
            if(isdigit(line[i]))
                left.push_back(line[i++]);
            else
                status = OPER;
            break;
        case OPER:
            op = line[i++];
            status = RIGHT;
            break;
        case RIGHT:
            if(isdigit(line[i]))
                right.push_back(line[i++]);
            break;
        
        default:
            break;
        }
    }

    Request req;
    std::cout << "left: " << left << std::endl;
    std::cout << "right: " << right << std::endl;
    std::cout << "op: " << op << std::endl;

    req._x = std::stoi(left);
    req._y = std::stoi(right);
    req._op = op;

    return req;
}

int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        usage(argv[0]);
        exit(USAGE_ERR);
    }

    std::string serverip = argv[1];
    uint16_t serverport = atoi(argv[2]);

    sock sock;
    sock.Socket();

    int n = sock.Connect(serverip,serverport);
    std::cout<<n<<std::endl;
    if(n != 0)
        return 1;

    std::string buff;

    while(true)
    {
        std::cout<<"Enter#";
        std::string line;
        getline(std::cin,line);

        Request req = ParseLine(line);

        //序列化
        std::string send_string;
        req.Serialize(&send_string);

        //添加报头
        send_string = AddHeader(send_string);
        //发送报文
        send(sock.Fd(),send_string.c_str(),send_string.size(),0);
        //读取报文
        std::string package;
    START:
        int n = ReadPackage(sock.Fd(),buff,&package);
        if(n == 0)
            goto START;
        else if(n == -1)
            break;
        else
        {}
        package = RemoveHeader(package,n);
        Response resp;
        resp.Deserialize(package);

        std::cout << "result: " << resp._result << "[code: " << resp._code << "]" << std::endl;
    }

    sock.Close();
    return 0;
}