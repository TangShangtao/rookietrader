#pragma once
#include <string>
#include <cstdint>
namespace rookietrader
{

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


};