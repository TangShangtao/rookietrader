//文件操作类
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
	static uint64_t read_file_content(const std::string& filename, std::string& content)
	{
		FILE* fp = fopen(filename.c_str(), "rb");
		fseek(fp, 0, SEEK_END);
		uint32_t length = ftell(fp);
		content.resize(length);
		fseek(fp, 0, SEEK_SET);
		fread((void*)content.data(), sizeof(char), length, fp);
		fclose(fp);
		return length;
	}

	static void write_file_content(const std::string& filename, const std::string& content)
	{
	}

	static void write_file_content(const std::string& filename, const void* data, std::size_t length)
	{
	}

	static bool exists(const std::string& filename)
	{
#if _WIN32
		int ret = _access(filename, 0);
#else
		int ret = access(filename.c_str(), 0);
#endif
		return ret == 0;
	}
};