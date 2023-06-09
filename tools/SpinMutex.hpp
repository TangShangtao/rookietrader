#pragma once
#include <atomic>
#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif



//多线程基础
//https://www.zhihu.com/search?type=content&q=C%2B%2B%E5%A4%9A%E7%BA%BF%E7%A8%8B
//内存序
//https://www.zhihu.com/search?type=content&q=%E5%86%85%E5%AD%98%E5%BA%8F

//实现自旋锁
class SpinMutex
{
private:
    std::atomic<bool> flag = { false };
public:
    void lock()
    {
        for (;;)
        {   //memory_order_acquire读必后
            if (!flag.exchange(true, std::memory_order_acquire)) break;
            //读完了
            while (flag.load(std::memory_order_relaxed))
            {
#ifdef _MSC_VER
                _mm_pause();
#else
                __builtin_ia32_pause();
#endif
            }
        }
    }
    void unlock()
    {
        //memory_order_release写必前
        flag.store(false, std::memory_order_release);
    }
};

//充当std::lock_guard的角色, 防止死锁
class SpinLock
{
public:
    //构造函数中上锁
	SpinLock(SpinMutex& mtx) :_mutex(mtx) { _mutex.lock(); }
    //删除拷贝构造函数以保证自旋锁唯一存在
	SpinLock(const SpinLock&) = delete;
    //删除赋值函数以保证自旋锁唯一存在
	SpinLock& operator=(const SpinLock&) = delete;
	//析构函数中释放锁
    ~SpinLock() { _mutex.unlock(); }

private:
	SpinMutex&	_mutex;
};