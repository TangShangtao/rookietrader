#pragma once
#include "Marcos.h"
//枚举类型定义
NS_BEGIN
//合约类型
typedef enum tagContractCategory
{
    CC_Stock,			//股票
    CC_Future,			//期货
    CC_FutOption,		//期货期权，商品期权是这个分类
    CC_Combination,		//组合
    CC_Spot,			//即期
    CC_EFP,				//期转现
    CC_SpotOption,		//现货期权，股指期权是这个分类
    CC_ETFOption,		//个股期权，ETF期权是这个分类

    CC_DC_Spot	= 20,	//数币现货
    CC_DC_Swap,			//数币永续
    CC_DC_Future,		//数币期货
    CC_DC_Margin,		//数币杠杆
    CC_DC_Option,		//数币期权

    CC_UserIndex = 90	//自定义指数
} ContractCategory;
//开平类型
typedef enum tagCoverMode
{
    CM_OpenCover,		//开平
    CM_CoverToday,		//开平昨平今
    CM_UNFINISHED,		//平未了结的
    CM_None			//不区分开平
} CoverMode;
//下单价格类型
typedef enum tagPriceMode
{
    PM_Both,		//市价限价都支持
    PM_Limit,		//只支持限价
    PM_Market,		//只支持市价
    PM_None	= 9		//不支持交易
} PriceMode;
//交易类型
typedef enum tagTradingMode
{
    TM_Both,	//多空都支持
    TM_Long,	//只能做多
    TM_LongT1,	//做多T+1
    TM_None = 9	//不能交易
} TradingMode;
//K线周期(min/5min/1h/day)
typedef enum tagBarPeriod
{
    BP_Tick,
    BP_Min,
    BP_5MIN,
    BP_DAY
    //BP_WEEK,
    //BP_MONTH
}BarPeriod;

NS_END;