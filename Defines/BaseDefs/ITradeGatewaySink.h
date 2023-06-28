#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/ConstantDefs/Types.h"
#include "DataKit/CommonMgr.h"
#include <stdint.h>

NS_BEGIN


class OrderInfo;
class TradeInfo;
class Error;
class Array;

//交易网关回报消息所调用的回调接口, 主要由GatewayAdapter实现, 用来将回报消息转发给结算模块、高频策略上下文
class ITradeGatewaySink
{
public:

	virtual CommonMgr*	get_cmgr() = 0;

public:

    //交易网关回报连接、断开连接时调用
	virtual void on_rsp_connect() = 0;
    //交易网关回报登录结果时调用
	virtual void on_rsp_login(bool bSucc, const char* msg, uint32_t tradingdate) = 0;
    //交易网关回报注销结果时调用
    virtual void on_rsp_logout(bool bSucc, const char* msg){}
    //交易网关回报已确认结算单信息后, 已经准备好可以下单时调用
    virtual void on_rsp_ready(bool bReady){}
    // //交易网关回报下单失败时回调
	// virtual void on_rsp_entrust_error(Entrust* entrust, Error *err){}

    //交易网关回报查询信息结果时调用
    virtual void on_rtn_query_account(Array* ayAccount){}
    virtual void on_rtn_query_positions(Array* ayPositions){}
    virtual void on_rtn_query_orders(Array* ayOrders){}
    virtual void on_rtn_query_trades(Array* ayTrades){}
    //交易网关回报订单状态更新时调用
	virtual void on_rtn_order(OrderInfo* orderInfo){}
    //交易网关回报最新成交时更新时调用
	virtual void on_rtn_trade(TradeInfo* tradeInfo){}
    //交易网关接收到错误信息时调用
    virtual void on_rsp_error(Error* err){}

};

NS_END;