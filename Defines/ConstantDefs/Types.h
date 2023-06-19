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
    BP_5Min,//5分钟的倍数，用5分钟bar重采样
    BP_Day
    //BP_WEEK,
    //BP_MONTH
}BarPeriod;

// //交易网关基础事件
// typedef enum tagGatewayEvent
// {
//     GE_Connect,         //与交易平台连接
//     GE_DisConnect,      //与交易平台断开连接
//     GE_Login,           //登录交易平台
//     GE_Logout           //登出交易平台
// }GatewayEvent;

//合约交易状态类型
typedef enum tagTradeStatus
{
    TS_BeforeTrading    = '0',  //开盘前
    TS_NotTrading       = '1',	//非交易
    TS_Continous        = '2',	//连续竞价
	TS_AuctionOrdering	= '3',	//集合竞价下单
	TS_AuctionBalance	= '4',	//集合竞价平衡
	TS_AuctionMatch		= '5',	//集合竞价撮合
	TS_Closed			= '6'	//收盘

}TradeStatus;
//交易网关连接状态
typedef enum tagTradingGatewayConnectState
{
    CS_NotInited,                //网关未初始化
    CS_DisConnected,            //未连接
    CS_Connecting,              //连接中
    CS_Connected,               //已连接未认证
    // CS_Authenticating,          //认证中
    // CS_Authenticated,           //已认证
    CS_Logging,                 //登录中
    CS_Logged,                  //已登录未确认结算单
    CS_Confirming,              //确认结算单中
    CS_Confirmed,               //已确认结算单
    CS_Ready                    //准备就绪
}TGConnectState;

//多空方向
typedef enum tagDirectionType
{
    DT_Long,
    DT_Short,
    DT_Net

}DirectionType;




NS_END;