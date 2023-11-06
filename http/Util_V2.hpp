#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include "log.hpp"

class Util
{
public:
    // 坑：一般网页文件，都是文本的
    // 但是如果是图片，视频，音频-> 二进制的
    static bool ReadFile(const std::string &path, std::string *fileContent)
    {
        // 1. 获取文件本身的大小
        struct stat st;
        int n = stat(path.c_str(), &st);
        if(n<0)return false;
        int size = st.st_size;
        // 2. 调整string的空间
        fileContent->resize(size);
        // 3. 读取
        int fd = open(path.c_str(), O_RDONLY);
        if(fd < 0) return false;
        read(fd, (char*)fileContent->c_str(), size);
        close(fd);
        logMessage(Info, "read file %s done", path.c_str());
        return true;
    }
    static std::string ReadOneLine(std::string &message, const std::string &sep)
    {
        auto pos = message.find(sep);
        if(pos == std::string::npos) return "";
        std::string s = message.substr(0, pos);
        message.erase(0, pos+sep.size());
        return s;
    }
    // GET /favicon.ico HTTP/1.1
    // GET /a/b/c/d/e/f/g/h/i/g/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y/z HTTP/1.1
    static bool ParseRequestLine(const std::string &line, std::string *method, std::string *url, std::string *httpVersion)
    {
        std::stringstream ss(line);
        ss >> *method >> *url >> *httpVersion;
        return true;
    }
};