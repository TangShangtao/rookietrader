
#include "../ConstantDefs/Marcos.h"
#include "../BaseDefs/BaseObject.hpp"

#include <unordered_map>
#include <vector>
#include <string>

NS_BEGIN


//哈希表数据结构定义
template <typename T>
class HashMap : public BaseObject
{
public:
    typedef typename std::unordered_map<T, BaseObject*> MapType;
    typedef typename MapType::const_iterator ConstIterator;

protected:
    MapType m_map;
public:
    static HashMap<T>* create()
    {
        HashMap<T>* pRet = new HashMap<T>();
        return pRet;
    }
    virtual void release() override
    {
        if (m_uRefs == 0)
        {
            return;
        }
        uint32_t cnt = m_uRefs.fetch_sub(1);
        if (cnt == 1)
        {
            clear();
            delete this;
        }
    }
    BaseObject* operator[](const T& key, bool bRetain=true)
    {
        auto it = m_map.find(key);
        if (it == m_map.end())
        {
            return nullptr;
        }
        if (bRetain)
        {
            it->second->retain();
        }
        return it->second;
    }

    ConstIterator begin() const
    {
        return m_map.begin();
    }
    ConstIterator end() const
    {
        return m_map.end();
    }
    ConstIterator find(const T& key) const
    {
        return m_map.find(key);
    }
    void clear()
    {
        ConstIterator it = m_map.begin();
        for (; it != m_map.end(); it++)
        {
            it->second->release();
        }
        m_map.clear();
    }
    void add(const T& key, BaseObject* obj)
    {
        if (obj)
        {
            obj->retain();
        }
        BaseObject* old = nullptr;
        auto it = m_map.find(key);
        if (it != m_map.end())
        {
            old = it->second;
        }
        if (old)
        {
            old->release();
        }
        m_map[key] = obj;
    }
    void remove(const T& key)
    {
        auto it = m_map.find(key);
        if (it != m_map.end())
        {
            it->second->release();
            m_map.erase(it);
        }
    }




protected:
    HashMap() {}
    virtual ~HashMap() {}

    
};


//配置信息数据结构定义
class Variant : public BaseObject
{
public:
    Variant* get(const char* key)
    {

        return nullptr;
    }

public:
    typedef enum
    {
        TYPE_NULL,
        TYPE_BOOL,
        TYPE_Int32,
        TYPE_UINT32,
        TYPE_INT64,
        TYPE_UINT64,
        TYPE_FLOAT,
        TYPE_DOUBLE,
        TYPE_STRING,
        TYPE_ARRAY,
    }ValueType;
    

};








NS_END;