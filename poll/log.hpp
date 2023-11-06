#pragma once
#include <string>
#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include <cstdarg>

const std::string filename = "./log/tpcserver.log";

enum 
{
    Debug = 0,
    Info,
    Warnning,
    Error,
    Fatal,
    UnKnown
};

static std::string toLevelString(int level)
{
    switch(level)
    {
        case Debug:
            return "Debug";
        case Info:
            return "Info";
        case Warnning:
            return "Wanning";
        case Error:
            return "Error";
        case Fatal:
            return "Fatal";
        default:
            return "UnKnown";
    }
}

static std::string getTime()
{
    time_t now = time(nullptr);
    struct tm* temp = localtime(&now);
    char buffer[128];
    snprintf(buffer,sizeof(buffer),"%d-%d-%d,%d:%d:%d",temp->tm_year+1900,temp->tm_mon+1,
                                    temp->tm_mday,temp->tm_hour,temp->tm_min,temp->tm_sec);

    return buffer;
}

void logMessage(int level,const char* format,...)
{
    char leftLog[1024];
    std::string level_string = toLevelString(level);
    std::string nowtime = getTime();

    snprintf(leftLog,sizeof(leftLog),"[%s],[%s],[%d]",level_string.c_str(),nowtime.c_str(),getpid());

    char rightLog[1024];
    va_list va;
    va_start(va,format);
    vsnprintf(rightLog,sizeof(rightLog),format,va);
    va_end(va);

    FILE* fp = fopen(filename.c_str(),"a");
    if(fp ==nullptr)
        return;

    fprintf(fp,"%s%s\n",leftLog,rightLog);

    fflush(fp);
    fclose(fp);
}