#include "CsvLoader.h"
#include "../Utils/FileUtils.hpp"
#include "../Utils/StrUtils.hpp"
CsvLoader::CsvLoader(const char* split /* = "," */) : m_split(split)
{

}
bool CsvLoader::load_from_file(const char* filename)
{
    if (!FileUtils::exists(filename)) return false;
    m_ifs.open(filename);
    m_ifs.getline(m_buffer, MAX_ROW_LENGTH);
    //TODO替换掉一些字段的特殊符号
    //TODO将字段名称转成小写
    std::vector<std::string> fields = StrUtils::split(m_buffer, m_split);
    for (uint32_t i = 0; i < fields.size(); i++)
    {
        std::string field = fields[i];
        if (field.empty()) break;
        m_fields_map[field] = i;
    }
    return true;
}
bool CsvLoader::next_row()
{
    if (m_ifs.eof()) return false;
    while (!m_ifs.eof())
    {
        m_ifs.getline(m_buffer, MAX_ROW_LENGTH);
        if (!strlen(m_buffer)) continue;
        else break;
    }
    if (!strlen(m_buffer)) return false;//文件剩余行全为空
    m_current_row.clear();
    m_current_row = StrUtils::split(m_buffer, m_split);
    return true;
}
