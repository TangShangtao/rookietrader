#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/ConstantDefs/Types.h"


NS_BEGIN


//vector + cursor封装一个数据容器，存放高频数据Tick，OrdDtl，OrdQue，Trans
template <typename T>
class HftDataList
{
public:
    std::string     _code;
    uint32_t        _date;
    std::size_t     _cursor;    //记录下一条数据的位置
    std::size_t     _count;

    std::vector<T>  _items;
    HftDataList() : _cursor(UINT_MAX), _count(0), _date(0) {} 
};
struct BarDataList
{
    std::string             _code;
    BarPeriod               _period;
    uint32_t                _cursor;
    uint32_t                _times;
    uint32_t                _count;
    
    std::vector<BarStruct>  _bars;
    // double                  _factor;//复权因子
    BarDataList() : _cursor(UINT32_MAX), _times(1), _count(0) {}
};

NS_END;