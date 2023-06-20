//动态库读取类
#pragma once
#include "tools/Logger.h"
#include <string>


#ifdef _MSC_VER
#include <wtypes.h>
typedef HMODULE		DllHandle;
typedef void*		ProcHandle;
#else
#include <dlfcn.h>
typedef void*		DllHandle;
typedef void*		ProcHandle;
#endif

class DLLHelper
{
public:
    //加载动态库, 返回其实例的指针(句柄)
    static DllHandle load_library(const char* filename)
    {
#ifdef _MSC_VER
        return ::LoadLibrary(filename);
#else
        DllHandle ret = dlopen(filename, RTLD_NOW);
        if (ret == NULL)
        {
            // printf("%s\n", dlerror());
            Logger::error("{}", dlerror());
        }
        return ret;
#endif
    }
    //释放动态库实例
    static void free_library(DllHandle handle)
    {
#ifdef _MSC_VER
		::FreeLibrary(handle);
#else
		dlclose(handle);
#endif
    }
    //根据符号名获得动态库中符号句柄
	static ProcHandle get_symbol(DllHandle handle, const char* name)
	{
		if (NULL == handle)
			return NULL;

#ifdef _MSC_VER
		return ::GetProcAddress(handle, name);
#else
		return dlsym(handle, name);
#endif
	}    
    //动态库名修改为动态库文件名
	static std::string module2filename(const char* name, const char* unixPrefix = "lib")
	{
#ifdef _WIN32
		std::string ret = name;
		ret += ".dll";
		return std::move(ret);
#else
		std::size_t idx = 0;
		while (!isalpha(name[idx]))
			idx++;
		std::string ret(name, idx);
		ret.append(unixPrefix);
		ret.append(name + idx);
		ret += ".so";
		return std::move(ret);
#endif
    }
};