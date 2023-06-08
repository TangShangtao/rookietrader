//字符串操作类
#pragma once 
#include <spdlog/fmt/fmt.h>
#include <string>

#define MAX_MSG_LEN 1024
class StrUtils
{
public:
    template<typename... Args>
    static char* format_to(char* buffer, const char* fmt, const Args& ...args)
    {
        char* s = fmt::format_to(buffer, fmt, args...);
        s[0] = '\0';
        return s;
    }
    template<typename... Args>
    static std::string format(const char* fmt, const Args& ...args)
    {
        std::string ret = fmt::format(fmt, args...);
        return ret;
    }
    //ignoreCase: 是否忽略大小写
	static inline bool endsWith(const char* str, const char* pattern, bool ignoreCase = true)
	{
		size_t thisLen = strlen(str);
		size_t patternLen = strlen(pattern);
		if (thisLen < patternLen || patternLen == 0)
			return false;

		const char* s = str + (thisLen - patternLen);

		if (ignoreCase)
		{
#ifdef _MSC_VER
			return _strnicmp(s, pattern, patternLen) == 0;
#else
			return strncasecmp(s, pattern, patternLen) == 0;
#endif
		}
		else
		{
			return strncmp(s, pattern, patternLen) == 0;
		}
	}
};