#include "Httpserver.hpp"
#include "Util.hpp"
#include <memory>
#include <vector>

const std::string SEP = "\r\n";
const std::string page_404 = "./wwwroot/err_404.html";

const std::string defaultHomepage = "index.html";
const std::string webRoot = "./wwwroot";

class HttpRequest
{
public:
    HttpRequest():_path(webRoot)
    {}

    void Print()
    {
        logMessage(Debug,"method:%s,url:%s,version:%s",_method.c_str(),_url.c_str(),_httpVersion.c_str());
        logMessage(Debug,"path:%s",_path.c_str());
        logMessage(Debug,"suffix:%s",_suffix.c_str());
    }

    ~HttpRequest()
    {}

public:
    std::string _method;
    std::string _url;
    std::string _httpVersion;
    std::vector<std::string> _body;
    std::string _path;
    std::string _suffix;
};

HttpRequest Deserialize(std::string& message)
{
    HttpRequest req;
    std::string line = Util::ReadOneLine(message,SEP);
    Util::ParseRequestLine(line,&req._method,&req._url,&req._httpVersion);

    while(!message.empty())
    {
        line = Util::ReadOneLine(message,SEP);
        req._body.push_back(line);
    }

    req._path += req._url;
    if(req._path[req._path.size()-1] == '/')
        req._path += defaultHomepage;

    auto pos = req._url.rfind(".");
    if(pos == std::string::npos)
        req._suffix = ".html";
    else
        req._suffix += req._path.substr(pos);

    return req;

}

void Usage(char* s)
{
    std::cout<<s<<" serverport"<<std::endl;
}

std::string GetContentType(const std::string& suffix)
{
    std::string content_type = "Content_Type: ";
    if(suffix == ".html" || suffix == ".htm")
        content_type += "text/html";
    else if(suffix == ".css")
        content_type += "text/css";
    else if(suffix == ".js")
        content_type += "application/x-javascript";
    else if(suffix == ".png")
        content_type += "image/png";
    else if(suffix == ".jpg")
        content_type += "image/jpg";
    else
    {}

    return content_type + SEP;
}

std::string HandlerHttp(std::string& message)
{
    std::cout<<"-------------------------------------"<<std::endl;
    std::cout<<message<<std::endl;

    HttpRequest req = Deserialize(message);
    req.Print();

    std::string body;
    std::string respond;
    if(true == Util::ReadFile(req._path,&body))
    {
        respond += "HTTP/1.0 500 OK" + SEP;
        respond += "Content-Length: " + std::to_string(body.size()) + SEP;
        respond += GetContentType(req._suffix);
        respond += SEP;
        respond += body;
    }
    else
    {
        respond += "HTTP/1.0 404 NOT FOUND" + SEP;
        Util::ReadFile(page_404,&body);
        respond += "Content-Length: " + std::to_string(body.size()) + SEP;
        respond += GetContentType(".html");
        respond += SEP;
        respond += body;
    }

    return respond;
}

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }

    uint16_t port = atoi(argv[1]);
    std::unique_ptr<HttpServer> tsvr(new HttpServer(port,HandlerHttp));
    tsvr->InitServer();
    tsvr->start();
    return 0;
}