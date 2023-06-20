//日志输出
//日志存储:文本
//日志内容:时间、线程ID、日志级别、行号、日志内容
//日志级别:DEBUG、INFO、WARNNING、ERROR、FATAL
//日志翻滚:设置日志大小
#pragma once
#include "Utils/StrUtils.hpp"

#include <memory>
#include <spdlog/spdlog.h>


#define MAX_MSG_LEN 1024
class Logger
{
public:
    typedef enum
    {
        LL_DEBUG,
        LL_INFO,
        LL_WARNNING,
        LL_ERROR,
        LL_FATAL
    }LogLevel;
    typedef std::shared_ptr<spdlog::logger> LoggerPtr;
public:
    static bool m_bInited;
    static LoggerPtr m_pLogger;
    // thread_local
    static char m_msg[MAX_MSG_LEN];
public:
    static LoggerPtr get_logger(const char* logger_name);
public:
    static void init();
    template<typename... Args>
    static void debug(const char* fmt, const Args& ...args)
    {
        if (!m_bInited)
        {
            init();
        }
        StrUtils::format_to(m_msg, fmt, args...);
        m_pLogger->debug(m_msg);
    }
    template<typename... Args>
    static void info(const char* fmt, const Args& ...args)
    {
        if (!m_bInited)
        {
            init();
        }
        StrUtils::format_to(m_msg, fmt, args...);
        m_pLogger->info(m_msg);
    }
    template<typename... Args>
    static void error(const char* fmt, const Args& ...args)
    {
        if (!m_bInited)
        {
            init();
        }
        StrUtils::format_to(m_msg, fmt, args...);
        m_pLogger->error(m_msg);
    }
    template<typename... Args>
    static void warn(const char* fmt, const Args& ...args)
    {
        if (!m_bInited)
        {
            init();
        }
        StrUtils::format_to(m_msg, fmt, args...);
        m_pLogger->warn(m_msg);
    }


};