#pragma once
#include <sys/stat.h>
#include <iostream>
#include <fcntl.h>
#include <sstream>
#include "log.hpp"

class Util
{
public:
    static bool ReadFile(const std::string &path,std::string *fileContent)
    {
        struct stat st;
        int n = stat(path.c_str(),&st);
        if(n == -1) return false;

        int size = st.st_size;
        fileContent->resize(size);

        int fd = open(path.c_str(),O_RDONLY);
        if(fd < 0)return false;

        read(fd,(char*)fileContent->c_str(),size);
        close(fd);
        logMessage(Info, "read file %s done", path.c_str());
        return true;
    }

    static std::string ReadOneLine(std::string& line,const std::string& sep)
    {
        auto pos = line.find(sep);
        if(pos == std::string::npos) return "";
        std::string s = line.substr(0,pos);
        line.erase(0,pos + sep.size());

        return s;
    }

    static bool ParseRequestLine(const std::string &line,std::string* method,std::string* url,std::string* httpVersion)
    {
        std::stringstream ss(line);
        ss>>*method>>*url>>*httpVersion;
        return true;
    }
};