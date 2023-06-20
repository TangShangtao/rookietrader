#pragma once
#include "Defines/ConstantDefs/Types.h"

#include <string.h>
#include <stdint.h>
NS_BEGIN
#pragma pack(push, 8)
struct TickStruct
{
public:
    char exchg[MAX_EXCHANGE_LENGTH];
    char code[MAX_INSTRUMENT_LENGTH];// CFFEX.IF.HOT
	double		price;				//最新价
	double		open;				//开盘价
	double		high;				//最高价
	double		low;				//最低价
	double		settle_price;		//结算价

	double		upper_limit;		//涨停价
	double		lower_limit;		//跌停价

	double		total_volume;		//总成交量
	double		volume;				//成交量
	double		total_turnover;		//总成交额
	double		turn_over;			//成交额
	double		open_interest;		//总持
	double		diff_interest;		//增仓

	uint32_t	trading_date;		//交易日,如20140327
	uint32_t	action_date;		//自然日期,如20140327
	uint32_t	action_time;		//发生时间,精确到毫秒,如105932000
	uint32_t	reserve_;			//占位符

	double		pre_close;			//昨收价
	double		pre_settle;			//昨结算
	double		pre_interest;		//上日总持

	double		bid_prices[10];		//委买价格
	double		ask_prices[10];		//委卖价格
	double		bid_qty[10];		//委买量
	double		ask_qty[10];		//委卖量
	TickStruct()
	{
		memset(this, 0, sizeof(TickStruct));
	}    
    
};

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