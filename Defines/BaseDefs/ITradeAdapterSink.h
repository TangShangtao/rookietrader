//继承此接口的类，可以作为交易适配器的回调接口, 包括:
//高频策略的策略运行上下文(wondertrader)
//执行器(wondertrader)
//结算模块?
#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include <stdint.h>
NS_BEGIN
class ITradeAdapterSink
{
public:
	/*
	 *	成交回报
	 */
	virtual void on_trade(uint32_t localid, const char* stdCode, bool isBuy, double vol, double price) = 0;

	/*
	 *	订单回报
	 */
	virtual void on_order(uint32_t localid, const char* stdCode, bool isBuy, double totalQty, double leftQty, double price, bool isCanceled = false) = 0;

	/*
	 *	持仓更新回调
	 */
	virtual void on_position(const char* stdCode, bool isLong, double prevol, double preavail, double newvol, double newavail, uint32_t tradingday) {}

	/*
	 *	交易通道就绪
	 */
	virtual void on_channel_ready() = 0;

	/*
	 *	交易通道丢失
	 */
	virtual void on_channel_lost() = 0;

	/*
	 *	下单回报
	 */
	virtual void on_entrust(uint32_t localid, const char* stdCode, bool bSuccess, const char* message){}

	/*
	 *	资金回调
	 */
	virtual void on_account(const char* currency, double prebalance, double balance, double dynbalance, double avaliable, double closeprofit, double dynprofit, double margin, double fee, double deposit, double withdraw){}
};



NS_END;