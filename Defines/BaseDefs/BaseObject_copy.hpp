//对象基类，引用计数
#pragma once
#include <stdint.h>
#include <atomic>

#include "../ConstantDefs/Marcos.h"

#include <iostream>
NS_BEGIN
class BaseObject
{
public:
    BaseObject() :m_uRefs(1) {std::cout << "BaseObject ctor, 地址: " << this << std::endl;}
    virtual ~BaseObject() {std::cout << "BaseObject dtor, 地址: " << this << std::endl;}

public:
    //
    uint32_t retain() 
    { 
        return m_uRefs.fetch_add(1) + 1;
    }
    virtual void release()
    {
        std::cout << "BaseObject release" << std::endl;
        if (m_uRefs == 0)
        {
            return;
        }
        uint32_t cnt = m_uRefs.fetch_sub(1);
        if (cnt == 1)
        {
            delete this;
        }
        
    }
    uint32_t getRefCount() { return m_uRefs; }

protected:
    volatile std::atomic<uint32_t> m_uRefs;
};
NS_END;