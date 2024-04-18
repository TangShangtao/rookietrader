#include "tools/utils.h"
#include <chrono>
#include <iomanip>
#include <string>

namespace rookietrader
{

std::string TimeUtils::GetCurrTradingDay()
{
    return "";
}

std::string TimeUtils::GetNextTradingDay()
{
    return "";
}

std::string TimeUtils::GetCurrDate()
{
    return "";
}

std::string TimeUtils::GetNextDate()
{
    return "";
}
std::string TimeUtils::GetTimeNow()
{
    
    auto now = std::chrono::system_clock::now();

    std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm local_time = *std::localtime(&time);

    std::stringstream ss;
    ss  <<  std::put_time(&local_time, "%H:%M:%S.") 
        <<  std::setfill('0') << std::setw(3) 
        <<  std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

    return ss.str();
}

};