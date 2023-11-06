#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include "Util.hpp"

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
int ParsePackage(std::string &inbuffer, std::string *package)
{
    auto pos = inbuffer.find(HEADER_SEP);
    if (pos == std::string::npos)
        return 0;

    std::string strlen = inbuffer.substr(0, pos);
    int len = Util::toInt(strlen);
    int targetPackage = len + strlen.size() + 2 * HEADER_SEP_LEN;
    if (targetPackage > inbuffer.size())
        return 0;

    *package = inbuffer.substr(0, targetPackage);
    inbuffer.erase(0, targetPackage);

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
        std::string x_string = std::to_string(_x);
        std::string y_string = std::to_string(_y);

        *outstr = x_string + SEP + _op + SEP + y_string;
        std::cout << "Request Serialize:\n"
                  << *outstr << std::endl;
    }

    bool Deserialize(const std::string &instr)
    {
        std::vector<std::string> result;
        Util::StringSpilt(instr, SEP, &result);
        if (result.size() != 3)
            return false;

        if (result[1].size() != 1)
            return false;

        _x = Util::toInt(result[0]);
        _y = Util::toInt(result[2]);
        _op = result[1][0];
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

    void Serialize(std::string *instr)
    {
        *instr = "";

        std::string result_string = std::to_string(_result);
        std::string code_string = std::to_string(_code);

        *instr = result_string + SEP + code_string;

        std::cout << "Response Serialize:\n"
                  << *instr << std::endl;
    }

    bool Deserialize(const std::string &instr)
    {
        std::vector<std::string> result;
        Util::StringSpilt(instr, SEP, &result);
        if (result.size() != 2)
            return false;

        _result = Util::toInt(result[0]);
        _code = Util::toInt(result[1]);

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