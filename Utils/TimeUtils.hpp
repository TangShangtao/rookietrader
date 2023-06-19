#pragma once
#include <sys/timeb.h>
#include <stdint.h>
#include <string>
#include <stdio.h>
#include <string.h>

//时间计算的封装, 不包含windows
class TimeUtils
{
public:
    //从1970年1月1日00:00:00 UTC开始计算的毫秒数
    static inline int64_t get_local_time_now()
    {
        timeb now;
        ftime(&now);//秒数
        return now.time * 1000 + now.millitm;//毫秒数
    }
    //得到当前时间的字符串, 格式为: 2018-01-01 00:00:00,000
    static inline std::string get_local_time_str(bool bIncludeMilliSec = true)
    {//为什么要返回std::string?
		uint64_t ltime = get_local_time_now();
		time_t now = ltime / 1000;
		uint32_t millitm = ltime % 1000;
		tm * tNow = localtime(&now);

		char str[64] = {0};
		if(bIncludeMilliSec)
			sprintf(str, "%02d:%02d:%02d,%03d", tNow->tm_hour, tNow->tm_min, tNow->tm_sec, millitm);
		else
			sprintf(str, "%02d:%02d:%02d", tNow->tm_hour, tNow->tm_min, tNow->tm_sec);
		return str;
    }
    //得到当前时间的整数, 格式为: 20180101
    static inline uint32_t get_cur_date_int()
    {
        uint64_t ltime = get_local_time_now();
        time_t now = ltime / 1000;
        tm * tNow = localtime(&now);
        return (tNow->tm_year + 1900) * 10000 + (tNow->tm_mon + 1) * 100 + tNow->tm_mday;
    }
    //输入日期得到该日期是星期几, 输入0则表示当前日期
    static inline uint32_t get_weekday_int(uint32_t uDate = 0)
    {
    	time_t ts = 0;
		if(uDate == 0)
		{
			ts = get_local_time_now()/1000;
		}
		else
		{
			tm t;	
			memset(&t,0,sizeof(tm));
			t.tm_year = uDate/10000 - 1900;
			t.tm_mon = (uDate%10000)/100 - 1;
			t.tm_mday = uDate % 100;
			ts = mktime(&t);
		}

		tm * tNow = localtime(&ts);
	
		return tNow->tm_wday;
    }


};