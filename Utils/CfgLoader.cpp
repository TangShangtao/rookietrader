#include "CfgLoader.h"
#include "../Defines/Containers/Variant.hpp"
#include "FileUtils.hpp"
#include "StrUtil.hpp"

#include <rapidjson/document.h>
namespace rj = rapidjson;

Variant* CfgLoader::load_from_file(const char* filename)
{
    if (!FileUtils::exists) return nullptr;

    std::string content;
    FileUtils::read_file_content(filename, content);
    if (content.empty()) return nullptr;
    //自动检测&转换编码的逻辑-待实现
    if (StrUtil::endsWith(filename, ".json"))
    {
        return load_from_json(content.c_str());
    }
    else if (StrUtil::endsWith(filename, ".yaml"))
    {
        return load_from_yaml(content.c_str());
    }
    return nullptr;
}
bool json2variant(const rj::Value& root, Variant* params);
Variant* CfgLoader::load_from_json(const char* content)
{
    rj::Document root;
    root.Parse(content);
    if (root.HasParseError())
    {
        return nullptr;
    }
    Variant* pret = Variant::createMap();
    if (!json2variant(root, pret))
    {
        pret->release();
        return nullptr;
    }
    return pret;
}

Variant* CfgLoader::load_from_yaml(const char* content)
{
    return nullptr;
}

bool json2variant(const rj::Value& root, Variant* params)
{
	if (root.IsObject() && !params->isMap())
		return false;

	if (root.IsArray() && !params->isArray())
		return false;

	if (root.IsObject())
	{
		for (auto& m : root.GetObject())
		{
			const char* key = m.name.GetString();
			const rj::Value& item = m.value;
			switch (item.GetType())
			{
			case rj::kObjectType:
			{
				Variant* subMap = Variant::createMap();
				if (json2variant(item, subMap))
                {
                    params->append(key, subMap, false);
                }
			}
			break;
			case rj::kArrayType:
			{
				Variant* subAy = Variant::createArray();
				if (json2variant(item, subAy))
                {
                    params->append(key, subAy, false);
                }
			}
			break;
			case rj::kNumberType:
				if (item.IsInt())
					params->append(key, item.GetInt());
				else if (item.IsUint())
					params->append(key, item.GetUint());
				else if (item.IsInt64())
					params->append(key, item.GetInt64());
				else if (item.IsUint64())
					params->append(key, item.GetUint64());
				else if (item.IsDouble())
					params->append(key, item.GetDouble());
				break;
			case rj::kStringType:
				params->append(key, item.GetString());
				break;
			case rj::kTrueType:
			case rj::kFalseType:
				params->append(key, item.GetBool());
				break;
			}
		}
	}
	else
	{
		for (auto& item : root.GetArray())
		{
			switch (item.GetType())
			{
			case rj::kObjectType:
			{
				Variant* subMap = Variant::createMap();
				if (json2variant(item, subMap))
					params->append(subMap, false);
			}
			break;
			case rj::kArrayType:
			{
				Variant* subAy = Variant::createArray();
				if (json2variant(item, subAy))
					params->append(subAy, false);
			}
			break;
			case rj::kNumberType:
				if (item.IsInt())
					params->append(item.GetInt());
				else if (item.IsUint())
					params->append(item.GetUint());
				else if (item.IsInt64())
					params->append(item.GetInt64());
				else if (item.IsUint64())
					params->append(item.GetUint64());
				else if (item.IsDouble())
					params->append(item.GetDouble());
				break;
			case rj::kStringType:
				params->append(item.GetString());
				break;
			case rj::kTrueType:
			case rj::kFalseType:
				params->append(item.GetBool());
				break;
			}
		}
	}
	return true;
}
