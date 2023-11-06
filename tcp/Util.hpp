#pragma once 
#include <iostream>
#include <vector>
#include <string>

class Util
{
public:
    static bool StringSpilt(const std::string& str,const std::string& sep,std::vector<std::string>* result)
    {
        size_t start = 0;

        while(start < str.size())
        {
            size_t pos = str.find(sep,start);
            if(pos == std::string::npos)
                break;
        
        result->push_back(str.substr(start,pos-start));
        start = pos + sep.size();
        }
        if(start != std::string::npos) result->push_back(str.substr(start));
        
        return true;
    }

    static int toInt(const std::string& s)
    {
        return atoi(s.c_str());
    }
};