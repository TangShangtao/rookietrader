#include "tools/utils.h"
#include <chrono>
#include <iomanip>
#include <string>
#include <iconv.h>

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
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm tmTime;
    localtime_r(&time, &tmTime);
    std::ostringstream oss;
    oss << std::put_time(&tmTime, "%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << millisecs.count();
    return oss.str();
}
static int code_convert(const char* from_charset, const char* to_charset, char* inbuf, size_t inlen, char* outbuf, size_t outlen)
{
    iconv_t cd;
    char** pIn = &inbuf;
    char** pOut = &outbuf;
    cd = iconv_open(to_charset, from_charset);
    if (cd == 0) return -1;

    int res = iconv(cd, pIn, &inlen, pOut, &outlen);
    iconv_close(cd);
    return res;
}
std::string StringUtils::GBKToUTF8(const std::string& inStr)
{
    int len = inStr.size() * 2 + 1;
    char* buf = (char*)std::malloc(sizeof(char)*len);
    
    if (code_convert("gb2312", "utf-8", (char*)inStr.c_str(), inStr.size(), buf, len) >= 0)
    {
        std::string res;
        res.append(buf);
        return res;
    }
    else 
    {
        std::free(buf);
        return "";
    }
}

};