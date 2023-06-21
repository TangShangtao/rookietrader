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

//CTP交易网关连接状态
typedef enum tagTradingGatewayConnectState
{
    CS_NotInited,               //网关未初始化
    CS_DisConnected,            //未连接
    CS_Connected,               //已连接未登录
    CS_Authenticated,           //已认证未登录
    CS_Logged,                  //已登录未确认结算单
    CS_Queried,                 //已查询结算单未确认结算单
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

//开平方向
typedef enum tagOffsetType
{
    OT_Open,            //开仓
    OT_Close,           //平仓, 上期为平昨
    OT_CloseToday,      //平今
    OT_CloseYesterday,  //平昨
    OT_ForceClose,      //强平
}OffsetType;

//价格类型
typedef enum tagPriceType
{
    PT_AnyPrice,                //市价单
    PT_LimitPrice,              //限价单
    PT_BestPrice,               //最优价
    PT_LastPrice,               //最新价
    //以下对标CTP的价格类型
    PT_CTP_LastPlusOneTicks,    //最新价浮动上浮1个ticks
    PT_CTP_LastPlusTwoTicks,    //最新价浮动上浮2个ticks
    PT_CTP_LastPlusThreeTicks,  //最新价浮动上浮3个ticks
    PT_CTP_Ask1,                //卖一价
    PT_CTP_Ask1PlusOneTicks,    //卖一价浮动上浮1个ticks
    PT_CTP_Ask1PlusTwoTicks,    //卖一价浮动上浮2个ticks
    PT_CTP_Ask1PlusThreeTicks,  //卖一价浮动上浮3个ticks
    PT_CTP_Bid1,                //买一价
    PT_CTP_Bid1PlusOneTicks,    //买一价浮动上浮1个ticks
    PT_CTP_Bid1PlusTwoTicks,    //买一价浮动上浮2个ticks
    PT_CTP_Bid1PlusThreeTicks,  //买一价浮动上浮3个ticks
    PT_CTP_FiveLevelPrice,      //五档价
    // //以下对标DC的价格类型
    // PT_DC_PostOnly,             //只做maker
    // PT_DC_Fok,                  //全部成交或立即取消
    // PT_DC_Ioc,                  //立即成交并取消剩余
    // PT_DC_OptLimitIoc           //市价委托立即成交并取消剩余
}PriceType;
//订单标志
typedef enum tagOrderFlag
{
    OF_Normal,          //普通订单
    OF_FOK,             //全部成交或立即取消
    OF_FAK,             //立即成交并取消剩余

}OrderFlag;
//业务类型
typedef enum tagBusinessType
{
	BT_CASH		= '0',	//普通买卖,
	BT_ETF		= '1',	//ETF申赎
	BT_EXECUTE	= '2',	//期权行权
	BT_QUOTE	= '3',	//期权报价
	BT_FORQUOTE = '4',	//期权询价
	BT_FREEZE	= '5',	//期权对锁
	BT_CREDIT	= '6',	//融资融券
	BT_UNKNOWN			//未知业务类型
}BusinessType;
//订单操作类型(撤单改单)
typedef enum tagActionFlag
{
    AF_Cancel,          //撤单
    AF_Modify,          //改单

}ActionFlag;
//订单状态
typedef enum tagOrderState
{
    OS_AllTraded,               //全部成交
    OS_PartTradedQueueing,      //部分成交还在队列中
    OS_PartTradedNotQueueing,   //部分成交不在队列中
    OS_NotTradeQueueing,        //未成交还在队列中
    OS_NotTradeNotQueueing,     //未成交不在队列中
    OS_Canceled,                //已撤单
    OS_Submitting,              //提交中
    OS_Cancelling,              //撤单中
    OS_NotTouched,              //尚未触发
}OrderState;
//订单类型
typedef enum tagOrderType
{
    OT_Normal,          //正常订单
    OT_Exception,       //异常订单
    OT_System,          //系统订单
    OT_Hedge,           //对冲订单

}OrderType;
typedef enum tagTradeType
{
	TT_Common				= '0',	//普通
	TT_OptionExecution		= '1',	//期权执行
	TT_OTC					= '2',	//OTC成交
	TT_EFPDerived			= '3',	//期转现衍生成交
	TT_CombinationDerived	= '4'	//组合衍生成交
}TradeType;










NS_END;