#pragma once
#include "Defines/ConstantDefs/Types.h"

#include <string.h>
#include <stdint.h>
NS_BEGIN

#pragma pack(push, 8)
struct BarStruct
{
public:

	uint32_t	date;		//开始日期
	uint32_t	reserve_;	//占位符
	uint64_t	time;		//开始时间
	double		open;		//开
	double		high;		//高
	double		low;		//低
	double		close;		//收
	double		volume;	    //bar内成交量
    //double    turnover    //bar内成交额
    //double    open_interest//时间段结束时持仓量
	double		settle;		//结算
	double		money;		//成交金额
	double		hold;	//总持
	double		add;	//增仓

public:
    BarStruct()
    {
        memset(this, 0, sizeof(BarStruct));;
    }
};

#pragma pack(pop)

NS_END;