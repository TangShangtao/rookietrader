//字符串操作类
#pragma once 
#include <spdlog/fmt/fmt.h>
#include <string>
#include <vector>

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
	//指定字符集，去除原字符首尾的空格、tab、换行
	static std::string trim(std::string& str, const char* delims = " \t\r", bool left = true, bool right = true)
	{
		std::string ret = str;
		if (right)
		{
			ret.erase(ret.find_last_not_of(delims) + 1);
		}
		if (left)
		{
			ret.erase(0, ret.find_first_not_of(delims));
		}
		return std::move(ret);//将局部变量ret直接"转移"到外部的左值，避免值拷贝
	}
	typedef std::vector<std::string> StringVector;
	static StringVector split(const std::string& str, const std::string& delims = "\t\n ", unsigned int maxSplits = 0)
	{
		StringVector ret;
		unsigned int numSplits = 0;

		// Use STL methods
		size_t start, pos;
		start = 0;
		do
		{
			pos = str.find_first_of(delims, start);
			if (pos == start)
			{
				ret.emplace_back("");
				// Do nothing
				start = pos + 1;
			}
			else if (pos == std::string::npos || (maxSplits && numSplits == maxSplits))
			{
				// Copy the rest of the std::string
				ret.emplace_back( str.substr(start) );
				break;
			}
			else
			{
				// Copy up to delimiter
				ret.emplace_back( str.substr(start, pos - start) );
				start = pos + 1;
			}
			// parse up to next real data
			//start = str.find_first_not_of(delims, start);
			++numSplits;

		} while (pos != std::string::npos);
		return std::move(ret);
	}
	static size_t my_strncpy(char* dest, const char* src, size_t len = 0)
	{
		if (dest == nullptr || src == nullptr)
			return -1;
		len = (len == 0) ? strlen(src) : len;
		memcpy(dest, src, len);
		dest[len] = '\0';
		return len;
	}
};