#pragma once
#include <string>
#include <cstdint>
#include <functional>
namespace rookietrader
{
class SignalHandler
{
public:
    typedef std::function<void(const std::string&)> SignalLogCallback;
    typedef std::function<void(int)> ExitCallback;
    static void PrintStackTrace(SignalLogCallback logCb);
    static void OnSystemSignal(int signum);
    static void RegisterSignalCallbacks(SignalLogCallback logCb, ExitCallback exitCb);
};
class TimeUtils
{
public:
    // format: 2000-07-21
    static std::string GetCurrTradingDay();
    // format: 2000-07-21
    static std::string GetNextTradingDay();
    // format: 2000-07-21
    static std::string GetCurrDate();
    // format: 2000-07-21
    static std::string GetNextDate();
    // format: 10:10:10.100
    static std::string GetTimeNow();
};

class StringUtils
{
public:
    static std::string GBKToUTF8(const std::string& inBuf);
};


};