#pragma once
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <string>
#include <memory>

namespace rookietrader
{
// log file output as csv
// log pattern: ClassName::FunctionName,log sentence1; log sentence2
class Logger
{
public:
    Logger(std::string loggerName, std::string logMode)
    {
        std::string logPath = "logs/log_" + loggerName + ".csv";
        fileLogger = spdlog::daily_logger_mt(loggerName + "File", logPath, 2, 30);
        consoleLogger = spdlog::stdout_color_mt(loggerName + "Console");
        std::string logPattern = "%H:%M:%S.%e,%n,%l,%t,%v,";
        fileLogger->set_pattern(logPattern);
        consoleLogger->set_pattern(logPattern);
        if (logMode == "debug")
        {
            fileLogger->set_level(spdlog::level::level_enum::debug);
            consoleLogger->set_level(spdlog::level::level_enum::debug);
        }
        else 
        {
            fileLogger->set_level(spdlog::level::level_enum::info);
            consoleLogger->set_level(spdlog::level::level_enum::info);
        }
    }
    Logger(const Logger&) = delete;
    ~Logger()
    {
        spdlog::drop_all();
    }
    template<typename T>
    void debug(T&& msg) const
    {
        fileLogger->debug(msg);
        consoleLogger->debug(msg);
    }
    template<typename T>
    void info(T&& msg) const
    {
        fileLogger->info(msg);
        consoleLogger->info(msg);
    }
    template<typename T>
    void warn(T&& msg) const
    {
        fileLogger->warn(msg);
        consoleLogger->warn(msg);
    }
    template<typename T>
    void error(T&& msg) const
    {
        fileLogger->error(msg);
        consoleLogger->error(msg);
    }  
    template<typename Arg1, typename... Args>
    void debug(const char* fmt, const Arg1& arg1, const Args&...args) const
    {
        fileLogger->debug(fmt, arg1, args...);
        consoleLogger->debug(fmt, arg1, args...);
    }
    template<typename Arg1, typename... Args>
    void info(const char* fmt, const Arg1& arg1, const Args&...args) const
    {
        fileLogger->info(fmt, arg1, args...);
        consoleLogger->info(fmt, arg1, args...);
    }
    template<typename Arg1, typename... Args>
    void warn(const char* fmt, const Arg1& arg1, const Args&...args) const
    {
        fileLogger->warn(fmt, arg1, args...);
        consoleLogger->warn(fmt, arg1, args...);
    }
    template<typename Arg1, typename... Args>
    void error(const char* fmt, const Arg1& arg1, const Args&...args) const
    {
        fileLogger->error(fmt, arg1, args...);
        consoleLogger->debug(fmt, arg1, args...);
    }    
private:
    std::shared_ptr<spdlog::logger> fileLogger;
    std::shared_ptr<spdlog::logger> consoleLogger;
};

};