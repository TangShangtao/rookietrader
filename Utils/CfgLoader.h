//配置文件加载器
#pragma once
#include "../Defines/ConstantDefs/Marcos.h"
#include "../Defines/Containers/Variant.hpp"
#include "../Defines/Containers/HashMap.hpp"
#include <string>

NS_BEGIN
class Variant;
NS_END;
USING_NS;

class CfgLoader
{
private:
    static Variant* load_from_json(const char* content);
    static Variant* load_from_yaml(const char* content);
public:
    static Variant* load_from_file(const char* filename);    

};

