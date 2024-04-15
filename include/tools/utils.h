#pragma once
#include <string>
#include <cstdint>
namespace rookietrader
{

class TimeUtils
{
public:
    static std::string GetCurrTradingDay();
    static std::string GetTimeNow();
    static uint32_t GetCurrMillisec();
};


};