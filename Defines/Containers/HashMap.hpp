#pragma once
#include "../ConstantDefs/Marcos.h"
#include "../BaseDefs/BaseObject.hpp"

#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

NS_BEGIN


//哈希表，存放对象基类的指针
template <typename T>
class HashMap : public BaseObject
{
public:
    typedef typename std::unordered_map<T, BaseObject*> MapType;
    typedef typename MapType::iterator Iterator;
    typedef typename MapType::const_iterator ConstIterator;

//private:
public:
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
    Iterator begin() 
    {
        return m_map.begin();
    }
    ConstIterator begin() const
    {
        return m_map.begin();
    }
    Iterator end() 
    {
        return m_map.end();
    }
    ConstIterator end() const
    {
        return m_map.end();
    }
    Iterator find(const T& key) 
    //不同地址同内容的const char*经过隐式转换后为同地址的const string
    {
        return m_map.find(key);
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
    BaseObject* get(const T& key, bool bRetain)
    {
        Iterator it = m_map.find(key);
        if (it == m_map.end())
        {
            return nullptr;
        }
        if (bRetain)
        {
            it->second->retain();
        }
        BaseObject* pret = it->second;
        return pret;
    }
    void add(const T& key, BaseObject* obj, bool bRetain)
    {
        if (obj && bRetain)
        {
            obj->retain();
        }
        BaseObject* old = nullptr;
        Iterator it = m_map.find(key);
        if (it != m_map.end())
        {
            old = it->second;
        }
        m_map[key] = obj;
        if (old)
        {
            old->release();
        }
        
    }
    uint32_t size() const
    {
        return (uint32_t)m_map.size();
    }
    void remove(const T& key)
    {
        Iterator it = m_map.find(key);
        if (it != m_map.end())
        {
            it->second->release();
            m_map.erase(it);
        }
    }
protected:
    HashMap() {}
    

    
};
NS_END;