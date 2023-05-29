//ÎÄ¼þ¸¨ÖúÀà
#pragma once

#include <stdint.h>
#include <string>
//#include <stdio.h>

#if _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

class StdFile
{
public:
	static inline uint64_t read_file_content(const char* filename, std::string& content)
	{
		FILE* f = fopen(filename, "rb");
		fseek(f, 0, SEEK_END);
		uint32_t length = ftell(f);
		content.resize(length);   // allocate memory for a buffer of appropriate dimension
		fseek(f, 0, 0);
		fread((void*)content.data(), sizeof(char), length, f);
		fclose(f);
		return length;
	}

	static inline void write_file_content(const char* filename, const std::string& content)
	{
		FILE* f = fopen(filename, "wb");
		fwrite((void*)content.data(), sizeof(char), content.size(), f);
		fclose(f);
	}

	static inline void write_file_content(const char* filename, const void* data, std::size_t length)
	{
		FILE* f = fopen(filename, "wb");
		fwrite(data, sizeof(char), length, f);
		fclose(f);
	}

	static inline bool exists(const char* filename)
	{
#if _WIN32
		int ret = _access(filename, 0);
#else
		int ret = access(filename, 0);
#endif
		return ret == 0;
	}
};