//文件操作类
//文件编码方式:windows下默认为gbk, linux下默认为utf-8
#pragma once

#include <stdint.h>
#include <stdio.h>
// #include <string>
#if _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

class FileUtils
{
public:
	//读取整个文件到string中
	static uint64_t read_file_content(const char* filename, std::string& content)
	{
		FILE* fp = fopen(filename, "rb");
		fseek(fp, 0, SEEK_END);
		uint32_t length = ftell(fp);
		content.resize(length);
		fseek(fp, 0, SEEK_SET);
		fread((void*)content.data(), sizeof(char), length, fp);
		fclose(fp);
		return length;
	}

	static void write_file_content(const char* filename, const std::string& content)
	{
	}

	static void write_file_content(const char* filename, const void* data, std::size_t length)
	{
	}
	//指定文件是否存在
	static bool exists(const char* filename)
	{
#if _WIN32
		int ret = _access(filename, 0);
#else
		int ret = access(filename, 0);
#endif
		return ret == 0;
	}
};