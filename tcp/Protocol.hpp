#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <jsoncpp/json/json.h>
#include "Util.hpp"
#include "log.hpp"


#define SEP " "
#define SEP_LEN strlen(SEP)
#define HEADER_SEP "\r\n"
const int HEADER_SEP_LEN = strlen(HEADER_SEP);

std::string AddHeader(const std::string &s)
{
    std::cout << "AddHeader 之前:\n"
              << s << std::endl;
    std::string str = std::to_string(s.size());
    str += HEADER_SEP;
    str += s;
    str += HEADER_SEP;

    std::cout << "AddHeader 之后:\n"
              << str << std::endl;
    return str;
}

//"7"\r\n""10 + 20"\r\n  => 10 + 20
std::string RemoveHeader(const std::string &s, int len)
{
    std::cout << "AddHeader 之前:\n"
              << s << std::endl;

    std::string res = s.substr(s.size() - HEADER_SEP_LEN - len, len);

    std::cout << "RemoveHeader 之后:\n"
              << res << std::endl;

    return res;
}

// 例如"7"\r\n""10 + 20"\r\n
int ReadPackage(int sock, std::string &inbuffer, std::string *package)
{
    std::cout << "ReadPackage inbuffer 之前:\n"
              << inbuffer << std::endl;
    char buffer[1024];
    ssize_t n = recv(sock, buffer, sizeof(buffer - 1), 0);
    if (n <= 0)
        return -1;

    buffer[n] = 0;
    inbuffer += buffer;

    auto pos = inbuffer.find(HEADER_SEP);
    if (pos == std::string::npos)
    {
        logMessage(Debug, "there is npos!");
        return 0;
    }

    std::cout << inbuffer << std::endl;
    std::string strlen = inbuffer.substr(0, pos);
    int len = Util::toInt(strlen);
    int targetPackage = len + strlen.size() + 2 * HEADER_SEP_LEN;
    logMessage(Debug, "targetPackage:%d, and inbuffer.size():%d", targetPackage, inbuffer.size());
    if (targetPackage > inbuffer.size())
    {
        logMessage(Debug, "targetPackage bigger!");
        return 0;
    }   

    *package = inbuffer.substr(0, targetPackage);
    inbuffer.erase(0, targetPackage);

    logMessage(Debug, "End of the func,and return len:%d", len);
    std::cout << "ReadPackage inbuffer 之后:\n"
              << inbuffer << std::endl;
    return len;
}

class Request
{
public:
    Request()
    {
    }

    Request(int x, int y, char op) : _x(x), _y(y), _op(op)
    {
    }

    void Serialize(std::string *outstr)
    {
        Json::Value root;
        root["x"] = _x;
        root["y"] = _y;
        root["op"] = _op;

        Json::FastWriter writer;
        *outstr = writer.write(root);
    }

    bool Deserialize(const std::string &instr)
    {
        Json::Value root;
        Json::Reader reader;
        reader.parse(instr,root);
        _x = root["x"].asInt();
        _y = root["y"].asInt();
        _op = root["op"].asInt();

        Print();
        return true;
    }

    void Print()
    {
        std::cout << "_x: " << _x << std::endl;
        std::cout << "_y: " << _y << std::endl;
        std::cout << "_z: " << _op << std::endl;
    }

    ~Request() {}

    int _x;
    int _y;
    char _op;
};

class Response
{
public:
    Response()
    {
    }

    Response(int result, int code) : _result(result), _code(code)
    {
    }

    void Serialize(std::string *outstr)
    {
        Json::Value root;
        Json::FastWriter writer;

        root["result"] = _result;
        root["code"] = _code;
        *outstr = writer.write(root);

    }

    bool Deserialize(const std::string &instr)
    {
        Json::Value root;
        Json::Reader reader;

        reader.parse(instr,root);
        _result = root["root"].asInt();
        _code = root["code"].asInt();

        Print();
        return true;
    }

    void Print()
    {
        std::cout << "_result: " << _result << std::endl;
        std::cout << "_code: " << _code << std::endl;
    }

    ~Response()
    {
    }

    int _result;
    int _code;
};