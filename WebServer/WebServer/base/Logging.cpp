// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "Logging.h"
#include "CurrentThread.h"
#include "Thread.h"
#include "AsyncLogging.h"
#include <assert.h>
#include <iostream>
#include <time.h>  
#include <sys/time.h> 


static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger_;

std::string Logger::logFileName_ = "./WebServer.log";

void once_init()
{
    AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
    AsyncLogger_->start(); 
}

void output(const char* msg, int len)
{
    pthread_once(&once_control_, once_init);
    AsyncLogger_->append(msg, len);
}

Logger::Impl::Impl(const char *fileName, int line)
  : stream_(),
    line_(line),
    basename_(fileName)
{
    formatTime();
}

void Logger::Impl::formatTime()
{
    /*
    struct timeval
    {
    __time_t tv_sec;        /* Seconds. *//*
    __suseconds_t tv_usec;  /* Microseconds. *//*
    };
    tv_sec为Epoch到创建struct timeval时的秒数，tv_usec为微秒数，即秒后面的零头。
    */
    struct timeval tv;
    time_t time;
    char str_t[26] = {0};
    /*int gettimeofday(struct  timeval*tv,struct  timezone *tz )
    gettimeofday()会把目前的时间用tv结构体返回，当地时区的信息则放到tz所指的结构中
    */
    gettimeofday (&tv, NULL);
    time = tv.tv_sec;
    //C 库函数 struct tm *localtime(const time_t *timer) 
    //使用 timer 的值来填充 tm 结构。timer 的值被分解为 tm 结构，并用本地时区表示。
    //struct tm {
    //int tm_sec;         /* 秒，范围从 0 到 59                */
    //int tm_min;         /* 分，范围从 0 到 59                */
    //int tm_hour;        /* 小时，范围从 0 到 23                */
    //int tm_mday;        /* 一月中的第几天，范围从 1 到 31                    */
    //int tm_mon;         /* 月份，范围从 0 到 11                */
    //int tm_year;        /* 自 1900 起的年数                */
    //int tm_wday;        /* 一周中的第几天，范围从 0 到 6                */
    //int tm_yday;        /* 一年中的第几天，范围从 0 到 365                    */
    //int tm_isdst;       /* 夏令时                        */    
    //};
    struct tm* p_time = localtime(&time);   
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
    stream_ << str_t;
}

Logger::Logger(const char *fileName, int line)
  : impl_(fileName, line)
{ }

Logger::~Logger()
{
    impl_.stream_ << " -- " << impl_.basename_ << ':' << impl_.line_ << '\n';
    const LogStream::Buffer& buf(stream().buffer());
    output(buf.data(), buf.length());
}