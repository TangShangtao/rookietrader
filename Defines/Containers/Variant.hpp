//存放配置信息的数据结构
#pragma once
#include "../ConstantDefs/Marcos.h"
#include "../BaseDefs/BaseObject.hpp"
#include "../ConstantDefs/Marcos.h"
#include "HashMap.hpp"

#include <unordered_map>
#include <vector>
#include <string>


NS_BEGIN

//json格式配置信息，如:mocker: cta 或slippage: 1
//1.根节点:
//值:Value._string(如cta，1)/Value._array，其中数字、字符串、布尔值都是以string存放
//Value:std::string*/
//2.叶节点:
//键:const char*，值:Variant*
//Value:HashMap<const char*>*
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
        // const std::vector<std::string>* _array;
        HashMap<const char*>* _map;
    }Value;

private:
    ValueType m_type;
    Value m_value;

public:
    static Variant* createMap()
    {
        Variant* pret = new Variant();
        pret->m_type = TYPE_MAP;
        pret->m_value._map = rookie::HashMap<const char*>::create();
        return pret;
    }
    // static Variant* createArray()
    // {
	// 	Variant* pret = new Variant();
	// 	pret->m_type = TYPE_ARRAY;
	// 	pret->m_value._array = new std::vector<std::string>();
	// 	return pret;
    // }
	virtual void release() override
	{
		if (getRefCount() == 1)
		{
			switch (m_type)
			{
			case TYPE_ARRAY:
				/* code */
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
    bool append(const char* key, Variant* subv, bool bRetain)
	{
		if (!isMap())
		{
			return false;
		}
		m_value._map->add(key, subv, bRetain);
		return true;
	}
	bool append(const char* key, const char* str)
	{
		if (!isMap()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<const char*>::create();
		}
		Variant* subv = Variant::create(str);
		m_value._map->add(key, subv, false);
		return true;
	}
	bool append(const char* key, int32_t i32)
	{
		if (!isMap()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<const char*>::create();
		}
		Variant* subv = Variant::create(i32);
		m_value._map->add(key, subv, false);
		return true;
	}

	bool append(const char* key, uint32_t u32)
	{
		if (!isMap()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<const char*>::create();
		}
		Variant* subv = Variant::create(u32);
		m_value._map->add(key, subv, false);
		return true;
	}

	bool append(const char* key, int64_t i64)
	{
		if (!isMap()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<const char*>::create();
		}
		Variant* subv = Variant::create(i64);
		m_value._map->add(key, subv, false);
		return true;
	}

	bool append(const char* key, uint64_t u64)
	{
		if (!isMap()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<const char*>::create();
		}
		Variant* subv = Variant::create(u64);
		m_value._map->add(key, subv, false);
		return true;
	}

	bool append(const char* key, double d)
	{
		if (!isMap()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<const char*>::create();
		}
		Variant* subv = Variant::create(d);
		m_value._map->add(key, subv, false);
		return true;
	}

	bool append(const char* key, bool b)
	{
		if (!isMap()) return false;
		if (m_value._map == nullptr)
		{
			m_value._map = rookie::HashMap<const char*>::create();
		}
		Variant* subv = Variant::create(b);
		m_value._map->add(key, subv, false);
		return true;
	}

	// inline bool append(const char* _str)
	// {
	// 	if (_type != VT_Array)
	// 		return false;

	// 	if (_value._array == NULL)
	// 	{
	// 		_value._array = ChildrenArray::create();
	// 	}

	// 	WTSVariant* item = WTSVariant::create(_str);
	// 	_value._array->append(item, false);
	// 	//item->release();

	// 	return true;
	// }

	// inline bool append(int32_t _i32)
	// {
	// 	if (_type != VT_Array)
	// 		return false;

	// 	if (_value._array == NULL)
	// 	{
	// 		_value._array = ChildrenArray::create();
	// 	}

	// 	WTSVariant* item = WTSVariant::create(_i32);
	// 	_value._array->append(item, false);
	// 	//item->release();

	// 	return true;
	// }

	// inline bool append(uint32_t _u32)
	// {
	// 	if (_type != VT_Array)
	// 		return false;

	// 	if (_value._array == NULL)
	// 	{
	// 		_value._array = ChildrenArray::create();
	// 	}

	// 	WTSVariant* item = WTSVariant::create(_u32);
	// 	_value._array->append(item, false);
	// 	//item->release();

	// 	return true;
	// }

	// inline bool append(int64_t _i64)
	// {
	// 	if (_type != VT_Array)
	// 		return false;

	// 	if (_value._array == NULL)
	// 	{
	// 		_value._array = ChildrenArray::create();
	// 	}

	// 	WTSVariant* item = WTSVariant::create(_i64);
	// 	_value._array->append(item, false);
	// 	//item->release();

	// 	return true;
	// }

	// inline bool append(uint64_t _u64)
	// {
	// 	if (_type != VT_Array)
	// 		return false;

	// 	if (_value._array == NULL)
	// 	{
	// 		_value._array = ChildrenArray::create();
	// 	}

	// 	WTSVariant* item = WTSVariant::create(_u64);
	// 	_value._array->append(item, false);
	// 	//item->release();

	// 	return true;
	// }

	// inline bool append(double _real)
	// {
	// 	if (_type != VT_Array)
	// 		return false;

	// 	if (_value._array == NULL)
	// 	{
	// 		_value._array = ChildrenArray::create();
	// 	}

	// 	WTSVariant* item = WTSVariant::create(_real);
	// 	_value._array->append(item, false);
	// 	//item->release();

	// 	return true;
	// }

	// inline bool append(bool _bool)
	// {
	// 	if (_type != VT_Array)
	// 		return false;

	// 	if (_value._array == NULL)
	// 	{
	// 		_value._array = ChildrenArray::create();
	// 	}

	// 	WTSVariant* item = WTSVariant::create(_bool);
	// 	_value._array->append(item, false);
	// 	//item->release();

	// 	return true;
	// }

	// inline bool append(WTSVariant *item, bool bAutoRetain = true)
	// {
	// 	if (_type != VT_Array || NULL == item)
	// 		return false;

	// 	if (_value._array == NULL)
	// 	{
	// 		_value._array = ChildrenArray::create();
	// 	}

	// 	_value._array->append(item, bAutoRetain);

	// 	return true;
	// }	
	
	
	
	
	bool has(const char* key) const
    {
        if (m_type != TYPE_MAP)
        {
            return false;
        }
        auto it = m_value._map->find(key);
        if (it == m_value._map->end())
        {
            return false;
        }
        return true;
    }
    int32_t asInt32() const
    {
        if (m_type != TYPE_INT32)
        {
            return 0;
        }
        return m_value._string ? (int32_t)atof(m_value._string->c_str()): 0;
    }
    int32_t asUInt32() const
    {
        if (m_type != TYPE_UINT32)
        {
            return 0;
        }
        return m_value._string ? (uint32_t)atof(m_value._string->c_str()): 0;
    }
    int64_t asInt64() const
    {
        if (m_type != TYPE_INT64)
        {
            return 0;
        }
        return m_value._string ? strtoll(m_value._string->c_str(), nullptr, 10): 0;
    }    
    uint64_t asUInt64() const
    {
        if (m_type != TYPE_UINT64)
        {
            return 0;
        }
        return m_value._string ? strtoll(m_value._string->c_str(), nullptr, 10): 0;
    } 
    double asDouble() const
    {
        if (m_type != TYPE_DOUBLE)
        {
            return 0.0;
        }
        return m_value._string ? strtod(m_value._string->c_str(), nullptr): 0.0;
    }
    std::string asString() const
    {
        if (m_type != TYPE_STRING)
        {
            return "";
        }
        return m_value._string ? *m_value._string : "";
    }
    bool asBool() const
    {
        if (m_type != TYPE_BOOL)
        {
            return false;
        }
        return m_value._string ? (m_value._string->compare("true") == 0) : false;
    }
    Variant* get(const char* key) const
    {
        if (m_type != TYPE_MAP)
        {
            return nullptr;
        }
        return static_cast<Variant*>(m_value._map->get(key, false));
    }
	int32_t getInt32(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->asInt32();

		return 0;
	}
	uint32_t getUInt32(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->asUInt32();

		return 0;
	}

	int64_t getInt64(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->asInt64();

		return 0;
	}

	uint64_t getUInt64(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->asUInt64();

		return 0;
	}
	double getDouble(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->asDouble();

		return 0.0;
	}
	std::string getString(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->asString();

		return "";
	}
	bool getBool(const char* key) const
	{
		Variant* p = get(key);
		if (p)
			return p->asBool();

		return false;
	}
	ValueType type() const {return m_type;}
	bool isArray() const {return m_type == TYPE_ARRAY;}
	bool isMap() const {return m_type == TYPE_MAP;}	

    
private:
	Variant() :m_type(TYPE_NULL) {}
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