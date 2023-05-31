#pragma once
#include "../ConstantDefs/Marcos.h"
#include "../BaseDefs/BaseObject.hpp"
#include <algorithm>
#include <functional>
#include <vector>

NS_BEGIN
//数组，存放对象基类的指针
class Array : public BaseObject
{
public:
	/*
	 *	数组迭代器
	 */
	typedef std::vector<BaseObject*>::iterator Iterator;
	typedef std::vector<BaseObject*>::const_iterator ConstIterator;

	typedef std::vector<BaseObject*>::reverse_iterator ReverseIterator;
	typedef std::vector<BaseObject*>::const_reverse_iterator ConstReverseIterator;

	typedef std::function<bool(BaseObject*, BaseObject*)>	SortFunc;

protected:
	Array():_holding(false){}


	std::vector<BaseObject*>	_vec;
	std::atomic<bool>		_holding;

public:
	static Array* create()
	{
		Array* pRet = new Array();
		return pRet;
	}

	/*
	 *	读取数组长度
	 */
	uint32_t size() const{ return (uint32_t)_vec.size(); }

	/*
	 *	清空数组,并重新分配空间
	 *	调用该函数会预先分配长度
	 *	预先分配好的数据都是NULL
	 */
	void resize(uint32_t _size)
	{
		if(!_vec.empty())
			clear();

		_vec.resize(_size, NULL);
	}

	/*
	 *	读取数组指定位置的数据
	 *	对比grab接口,at接口只取得数据
	 *	不增加数据的引用计数
	 *	grab接口读取数据以后,增加引用计数
	 */
	BaseObject* at(uint32_t idx)
	{
		if(idx <0 || idx >= _vec.size())
			return NULL;

		BaseObject* pRet = _vec.at(idx);
		return pRet;
	}

	uint32_t idxOf(BaseObject* obj)
	{
		if (obj == NULL)
			return -1;

		uint32_t idx = 0;
		auto it = _vec.begin();
		for (; it != _vec.end(); it++, idx++)
		{
			if (obj == (*it))
				return idx;
		}

		return -1;
	}

	template<typename T> 
	T* at(uint32_t idx)
	{
		if(idx <0 || idx >= _vec.size())
			return NULL;

		BaseObject* pRet = _vec.at(idx);
		return static_cast<T*>(pRet);
	}

	/*
	 *	[]操作符重载
	 *	用法同at函数
	 */
	BaseObject* operator [](uint32_t idx)
	{
		if(idx <0 || idx >= _vec.size())
			return NULL;

		BaseObject* pRet = _vec.at(idx);
		return pRet;
	}

	/*
	 *	读取数组指定位置的数据
	 *	增加引用计数
	 */
	BaseObject*	grab(uint32_t idx)
	{
		if(idx <0 || idx >= _vec.size())
			return NULL;

		BaseObject* pRet = _vec.at(idx);
		if (pRet)
			pRet->retain();

		return pRet;
	}

	/*
	 *	数组末尾追加数据
	 *	数据自动增加引用计数
	 */
	void append(BaseObject* obj, bool bAutoRetain = true)
	{
		if (bAutoRetain && obj)
			obj->retain();

		_vec.emplace_back(obj);
	}

	/*
	 *	设置指定位置的数据
	 *	如果该位置已有数据,则释放掉
	 *	新数据引用计数增加
	 */
	void set(uint32_t idx, BaseObject* obj, bool bAutoRetain = true)
	{
		if(idx >= _vec.size() || obj == NULL)
			return;

		if(bAutoRetain)
			obj->retain();

		BaseObject* oldObj = _vec.at(idx);
		if(oldObj)
			oldObj->release();

		_vec[idx] = obj;
	}

	void append(Array* ay)
	{
		if(ay == NULL)
			return;

		_vec.insert(_vec.end(), ay->_vec.begin(), ay->_vec.end());
		ay->_vec.clear();
	}

	/*
	 *	数组清空
	 *	数组内所有数据释放引用
	 */
	void clear()
	{
		{
			std::vector<BaseObject*>::iterator it = _vec.begin();
			for (; it != _vec.end(); it++)
			{
				BaseObject* obj = (*it);
				if (obj)
					obj->release();
			}
		}
		
		_vec.clear();
	}

	/*
	 *	释放数组对象,用法如WTSObject
	 *	不同的是,如果引用计数为1时
	 *	释放所有数据
	 */
	virtual void release() override
	{
		if (m_uRefs == 0)
			return;

		try
		{
			m_uRefs--;
			if (m_uRefs == 0)
			{
				clear();
				delete this;
			}
		}
		catch(...)
		{

		}
	}

	/*
	 *	取得数组对象起始位置的迭代器
	 */
	Iterator begin()
	{
		return _vec.begin();
	}

	ConstIterator begin() const
	{
		return _vec.begin();
	}

	ReverseIterator rbegin()
	{
		return _vec.rbegin();
	}

	ConstReverseIterator rbegin() const
	{
		return _vec.rbegin();
	}

	/*
	 *	取得数组对象末尾位置的迭代器
	 */
	Iterator end()
	{
		return _vec.end();
	}

	ConstIterator end() const
	{
		return _vec.end();
	}

	ReverseIterator rend()
	{
		return _vec.rend();
	}

	ConstReverseIterator rend() const
	{
		return _vec.rend();
	}

	void	sort(SortFunc func)
	{
		std::sort(_vec.begin(), _vec.end(), func);
	}


};





NS_END;




