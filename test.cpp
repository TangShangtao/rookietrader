#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/Containers/HashMap.hpp"
#include "Defines/Containers/Variant.hpp"
#include "Utils/FileUtils.hpp"
#include "Utils/CfgLoader.h"
#include "Defines/BaseDefs/BaseObject.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <rapidjson/document.h>

#define print std::cout << 
#define endl << std::endl

USING_NS;
namespace rj = rapidjson;
int main()
{

    std::string content;
    FileUtils::read_file_content("dist/common/commodities.json", content);
    rj::Document root;
    root.Parse(content.c_str());
    if (root.HasParseError())
    {
        return 1;
    }
    Variant* map = CfgLoader::load_from_json(content.c_str());
    Variant* next = map->get("CFFEX");
    if (next == nullptr)
    {
        print "not find" endl;
    }
    else
    {
        print "found" endl;
    }
}