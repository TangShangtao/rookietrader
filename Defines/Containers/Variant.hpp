//存放配置信息的数据结构
//实现了增、查功能
#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/BaseDefs/BaseObject.hpp"
#include "HashMap.hpp"
#include "Array.hpp"
#include <unordered_map>
#include <vector>
#include <string>
//json格式配置信息，如:mocker: cta 或slippage: 1
//1.根节点:
//值:Value._string(如cta，1)/Value._array，其中数字、字符串、布尔值都是以string存放
//2.叶节点:
//键:const char*，值:Variant*
//Value:HashMap<const char*>*

//注1:键值(key)传递时以const char*类型, 存入哈希表时以std::string类型
NS_BEGIN

class Variant : public BaseObject
{
public:
    typedef enum
    {
        TYPE_NULL,
        TYPE_BOOL,
        TYPE_INT32,
        TYPE_UINT32,
        TYPE_INT64,
        TYPE_UINT64,
        TYPE_DOUBLE,
        TYPE_STRING,
        TYPE_ARRAY,
        TYPE_MAP,
    }ValueType;
    typedef union _value
    {
        const std::string* _string;
        Array* _array;
        HashMap<std::string>* _map;
    }Value;

//private:
public:
    ValueType m_type;
    Value m_value;

public:
    static Variant* create_map()
    {
        Variant* pret = new Variant();
        pret->m_type = TYPE_MAP;
        pret->m_value._map = rookie::HashMap<std::string>::create();
        return pret;
    }
    static Variant* create_array()
    {
		Variant* pret = new Variant();
		pret->m_type = TYPE_ARRAY;
		pret->m_value._array = Array::create();
		return pret;
    }
	virtual void release() override
	{
		if (get_ref_count() == 1)
		{
			switch (m_type)
			{
			case TYPE_ARRAY:
				if(m_value._array)
				{
					m_value._array->release();
				}
				break;
			case TYPE_MAP:
				if (m_value._map)
				{
					m_value._map->release();
				}
				break;
			default:
				if (m_value._string)
				{
					delete m_value._string;
				}
				break;
			}
		}
		BaseObject::release();
		
	}
	//添加键值对
    bool append(const char* key, Variant* subv, bool bRetain)
	{
		if (!is_map())
		{
			return false;
		}
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<std::string>::create();
		}
		m_value._map->add(key, subv, bRetain);
		return true;
	}
	bool append(const char* key, const char* str)
	{
		if (!is_map()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<std::string>::create();
		}
		Variant* subv = Variant::create(str);
		m_value._map->add(key, subv, false);
		return true;
	}
	bool append(const char* key, int32_t i32)
	{
		if (!is_map()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<std::string>::create();
		}
		Variant* subv = Variant::create(i32);
		m_value._map->add(key, subv, false);
		return true;
	}
	bool append(const char* key, uint32_t u32)
	{
		if (!is_map()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<std::string>::create();
		}
		Variant* subv = Variant::create(u32);
		m_value._map->add(key, subv, false);
		return true;
	}
	bool append(const char* key, int64_t i64)
	{
		if (!is_map()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<std::string>::create();
		}
		Variant* subv = Variant::create(i64);
		m_value._map->add(key, subv, false);
		return true;
	}
	bool append(const char* key, uint64_t u64)
	{
		if (!is_map()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<std::string>::create();
		}
		Variant* subv = Variant::create(u64);
		m_value._map->add(key, subv, false);
		return true;
	}
	bool append(const char* key, double d)
	{
		if (!is_map()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<std::string>::create();
		}
		Variant* subv = Variant::create(d);
		m_value._map->add(key, subv, false);
		return true;
	}
	bool append(const char* key, bool b)
	{
		if (!is_map()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<std::string>::create();
		}
		Variant* subv = Variant::create(b);
		m_value._map->add(key, subv, false);
		return true;
	}
	bool append(const char* _str)
	{
		if (m_type != TYPE_ARRAY)
			return false;

		if (!m_value._array)
		{
			m_value._array = Array::create();
		}
		Variant* subv = Variant::create(_str);
		m_value._array->append(subv, false);
		return true;
	}
	//数组添加元素
	bool append(Variant* subv, bool bRetain)
	{
		if (m_type != TYPE_ARRAY || subv == nullptr)
			return false;

		if (!m_value._array)
		{
			m_value._array = Array::create();
		}
		m_value._array->append(subv, bRetain);
		return true;
	}
	bool append(int32_t _i32)
	{
		if (m_type != TYPE_ARRAY)
			return false;

		if (!m_value._array)
		{
			m_value._array = Array::create();
		}
		Variant* subv = Variant::create(_i32);
		m_value._array->append(subv, false);
		return true;
	}
	bool append(uint32_t _u32)
	{
		if (m_type != TYPE_ARRAY)
			return false;

		if (!m_value._array)
		{
			m_value._array = Array::create();
		}
		Variant* subv = Variant::create(_u32);
		m_value._array->append(subv, false);
		return true;
	}
	bool append(int64_t _i64)
	{
		if (m_type != TYPE_ARRAY)
			return false;

		if (!m_value._array)
		{
			m_value._array = Array::create();
		}
		Variant* subv = Variant::create(_i64);
		m_value._array->append(subv, false);
		return true;
	}
	bool append(uint64_t _u64)
	{
		if (m_type != TYPE_ARRAY)
			return false;

		if (!m_value._array)
		{
			m_value._array = Array::create();
		}
		Variant* subv = Variant::create(_u64);
		m_value._array->append(subv, false);
		return true;
	}
	bool append(double _real)
	{
		if (m_type != TYPE_ARRAY)
			return false;

		if (!m_value._array)
		{
			m_value._array = Array::create();
		}
		Variant* subv = Variant::create(_real);
		m_value._array->append(subv, false);
		return true;
	}
	bool append(bool _bool)
	{
		if (m_type != TYPE_ARRAY)
			return false;

		if (!m_value._array)
		{
			m_value._array = Array::create();
		}
		Variant* subv = Variant::create(_bool);
		m_value._array->append(subv, false);
		return true;
	}
	uint32_t size() const
	{
		if (m_type != TYPE_ARRAY && m_type != TYPE_MAP)
		{
			return -1;
		}
		else if (m_type == TYPE_ARRAY)
		{
			return (!m_value._array) ? m_value._array->size() : 0;
		}
		else
		{
			return (!m_value._map) ? m_value._map->size() : 0;
		}

	}
	//判断是否存在键值对
	bool has(const char* key) const
    {
        if (m_type != TYPE_MAP)
        {
            return false;
        }
        if (m_value._map->find(key) == m_value._map->end())
        {
            return false;
        }
        return true;
    }
	//由键获取值, 不增加引用计数
	//注:接受键值的参数类型为std::string&，而不是const char*
    Variant* get(const std::string& key) const
    {
        if (m_type != TYPE_MAP)
        {
            return nullptr;
        }
        return static_cast<Variant*>(m_value._map->get(key, false));
    }
	//注:已经明确知道此处为基类指针转换为派生类指针，所以可以使用static_cast
	//对于Array
	Variant* get(uint32_t idx) const
	{
		if (m_type != TYPE_ARRAY || m_value._array == nullptr)
		{
			return nullptr;
		}
		Variant* ret = static_cast<Variant*>(m_value._array->at(idx));
		return ret;
	}
	//由键获取值, 不增加引用计数, 并直接值输出
	int32_t get_int32(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->as_int32();

		return 0;
	}
	uint32_t get_uint32(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->as_uint32();

		return 0;
	}

	int64_t get_int64(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->as_int64();

		return 0;
	}

	uint64_t get_uint64(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->as_uint64();

		return 0;
	}
	double get_double(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->as_double();

		return 0.0;
	}
	std::string get_string(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->as_string();

		return "";
	}
	const char* get_cstring(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->as_cstring();

		return "";
	}
	bool get_bool(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->as_bool();

		return false;
	}
	//获得全部键
	typedef std::vector<std::string> Keys;
	Keys get_keys() const
	{
		Keys ret;
		if (m_type == TYPE_MAP && m_value._map)
		{
			auto it = m_value._map->begin();
			for (; it != m_value._map->end(); it++)
			{
				ret.emplace_back(it->first);
			}
		}
		//return std::move(ret);
		return ret;
	}
	//根节点数据输出
    int32_t as_int32() const
    {
        if (m_type != TYPE_INT32)
        {
            return 0;
        }
        return m_value._string ? (int32_t)atof(m_value._string->c_str()): 0;
    }
    int32_t as_uint32() const
    {
        if (m_type != TYPE_UINT32)
        {
            return 0;
        }
        return m_value._string ? (uint32_t)atof(m_value._string->c_str()): 0;
    }
    int64_t as_int64() const
    {
        if (m_type != TYPE_INT64)
        {
            return 0;
        }
        return m_value._string ? strtoll(m_value._string->c_str(), nullptr, 10): 0;
    }    
    uint64_t as_uint64() const
    {
        if (m_type != TYPE_UINT64)
        {
            return 0;
        }
        return m_value._string ? strtoll(m_value._string->c_str(), nullptr, 10): 0;
    } 
    double as_double() const
    {
        if (m_type != TYPE_DOUBLE)
        {
            return 0.0;
        }
        return m_value._string ? strtod(m_value._string->c_str(), nullptr): 0.0;
    }
    std::string as_string() const
    {
        if (m_type != TYPE_STRING)
        {
            return "";
        }
        return m_value._string ? *m_value._string : "";
    }
	const char* as_cstring() const
	{
		if (m_type != TYPE_STRING)
		{
			return "";
		}
		return m_value._string ? m_value._string->c_str() : "";
	}
    bool as_bool() const
    {
        if (m_type != TYPE_BOOL)
        {
            return false;
        }
        return m_value._string ? (m_value._string->compare("true") == 0) : false;
    }

	//值类型对外接口
	ValueType type() const {return m_type;}
	bool is_array() const {return m_type == TYPE_ARRAY;}
	bool is_map() const {return m_type == TYPE_MAP;}	

public:    
// private:
	Variant() :m_type(TYPE_NULL) {}
	//创建根节点:ValueType -> string
    static Variant* create(int32_t i32)
    {
        Variant* pret = new Variant();
        pret->m_type = TYPE_INT32;
        char s[32] = {0};
        sprintf(s, "%d", i32);
        pret->m_value._string = new std::string(s);
        return pret;
    }
    static Variant* create(uint32_t u32)
    {
        Variant* pret = new Variant();
        pret->m_type = TYPE_UINT32;
        char s[32] = {0};
        sprintf(s, "%u", u32);
        pret->m_value._string = new std::string(s);
        return pret;
    }
    static Variant* create(int64_t i64)
    {
        Variant* pret = new Variant();
        pret->m_type = TYPE_INT64;
        char s[32] = {0};
        sprintf(s, "%ld", i64);
        pret->m_value._string = new std::string(s);
        return pret;
    }
    static Variant* create(uint64_t u64)
    {
        Variant* pret = new Variant();
        pret->m_type = TYPE_UINT64;
        char s[32] = {0};
        sprintf(s, "%lu", u64);
        pret->m_value._string = new std::string(s);
        return pret;
    }    
    static Variant* create(double d)
    {
        Variant* pret = new Variant();
        pret->m_type = TYPE_DOUBLE;
        char s[32] = {0};
        sprintf(s, "%.10f", d);
        pret->m_value._string = new std::string(s);
        return pret;
    }  
	static Variant* create(const char* str)
	{
		Variant* pret = new Variant();
		pret->m_type = TYPE_STRING;
		pret->m_value._string = new std::string(str);
		return pret;
	}
	static Variant* create(bool _bool)
	{
		Variant* pret = new Variant();
		pret->m_type = TYPE_BOOL;
		pret->m_value._string = new std::string(_bool ? "true" : "false");
		return pret;
	}
};
NS_END;