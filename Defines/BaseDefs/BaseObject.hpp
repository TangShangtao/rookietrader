//对象基类，引用计数
#pragma once
#include <stdint.h>
#include <atomic>
#include <boost/pool/pool.hpp>

#include "../ConstantDefs/Marcos.h"
#include "../../tools/SpinMutex.hpp"
NS_BEGIN
//基类对象，引用计数
class BaseObject
{
public:
    BaseObject() :m_uRefs(1) {}
    virtual ~BaseObject() {}

public:
    //
    uint32_t retain() 
    { 
        return m_uRefs.fetch_add(1) + 1;
    }
    virtual void release()
    {
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
    uint32_t get_ref_count() { return m_uRefs; }

protected:
    volatile std::atomic<uint32_t> m_uRefs;
};
//对象池
template<typename T>
class BasePool
{
private:
    boost::pool<> _pool;
public:
    BasePool() : _pool(sizeof(T)) {}
    virtual ~BasePool() {}

    T* construct()
    {
        void* mem = _pool.malloc();
        if (!mem) return nullptr;
        T* pobj = new(mem) T();
        return pobj;
    }
    void destroy(T* pobj)
    {
        pobj->~T();
        _pool.free(pobj);
    }
    void release()
    {
        _pool.release_memory();
    }
};
template<typename T>
class PoolObject : public BaseObject
{
public:
    typedef BasePool<T> MyPool;
public:
    MyPool* m_pool;
    SpinMutex* m_mutex;
public:
    PoolObject() : m_pool(nullptr) {}
    virtual ~PoolObject() {}
public:
    static T* allocate()
    {
        thread_local static MyPool pool;
        thread_local static SpinMutex mtx;
        mtx.lock();
        T* ret = pool.construct();
        mtx.unlock();
        ret->m_pool = &pool;
        ret->m_mutex = &mtx;
        return ret;
    }
public:
    virtual void release() override
    {
		if (m_uRefs == 0)
			return;

		try
		{
			uint32_t cnt = m_uRefs.fetch_sub(1);
			if (cnt == 1)
			{
				m_mutex->lock();
				m_pool->destroy((T*)this);
				m_mutex->unlock();
			}
		}
		catch (...)
		{

		}
	}   

};
NS_END;