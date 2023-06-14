#pragma once
#include <fstream>
#include <unordered_map>
#include <vector>

#define MAX_ROW_LENGTH 1024
class CsvLoader
{
private:
    std::ifstream m_ifs;
    char m_buffer[MAX_ROW_LENGTH];  //存放一行的原始数据
    std::string m_split;

    std::unordered_map<std::string, int32_t> m_fields_map; //表头{"open" : 1, "high" : 2}
    std::vector<std::string> m_current_row; //存放一行分割后的数据
public:
    CsvLoader(const char* split = ",");
public:
    bool load_from_file(const char* filename);
    bool next_row();
    //由列号获得当前row的值
	int32_t		get_int32(int32_t col);
	uint32_t	get_uint32(int32_t col);
	int64_t		get_int64(int32_t col);
	uint64_t	get_uint64(int32_t col);
	double		get_double(int32_t col);
	const char*	get_string(int32_t col);
    //由列名获得当前
	int32_t		get_int32(const char* field);
	uint32_t	get_uint32(const char* field);

	int64_t		get_int64(const char* field);
	uint64_t	get_uint64(const char* field);

	double		get_double(const char* field);

	const char*	get_string(const char* field);

};