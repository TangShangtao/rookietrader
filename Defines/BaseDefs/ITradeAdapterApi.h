//交易适配器提供给执行器、高频策略上下文的接口
#pragma once
#include "Defines/ConstantDefs/Marcos.h"

#include "Defines/BaseDefs/ITradeGatewayApi.h"
#include "Defines/BaseDefs/ITradeAdapterSink.h"

#include "DataKit/CommonMgr.h"

#include "Defines/BaseDefs/IExecuteContext.h"

#include <stdint.h>
NS_BEGIN
class Variant;


class ITradeAdapterApi
{
public:
    virtual ~ITradeAdapterApi() {}
public:
    //adapater基础设置
    virtual int init(const char* id, Variant* params, CommonMgr* bdMgr);
    virtual int init(const char* id, ITradeGatewayApi* api, CommonMgr* bdMgr);
    virtual int release();
    // virtual void register_sink(ITradeAdapterSink* listener);
    //准备交易
    virtual int   prepare_trade();
    virtual int   is_ready();
    //交易接口
    LocalOrderID  open_long(const char* stdCode, double price, double qty, OrderFlag flag);
    LocalOrderID  open_short(const char* stdCode, double price, double qty, OrderFlag flag);
    LocalOrderID  close_long(const char* stdCode, double price, double qty, bool isToday, OrderFlag flag);
    LocalOrderID  close_short(const char* stdCode, double price, double qty, bool isToday, OrderFlag flag);
    LocalOrderIDs buy(const char* stdCode, double price, double qty, int flag, bool bForceClose);
    LocalOrderIDs sell(const char* stdCode, double price, double qty, int flag, bool bForceClose);
    int           cancel(LocalOrderID localID);
    LocalOrderIDs cancel(const char* stdCode, bool isBuy, double qty);
    //查询接口
    double        query_position(const char* stdCode, bool bValidOnly, int32_t flag = 3);//?
    // double        get_orders(const char* stdCode);
    double        get_undone_qty(const char* stdCode);
    // //下单风控
    // bool check_cancel_limits(const char* stdCode);
    // bool check_order_limits(const char* stdCode);
    // bool check_self_match(const char* stdCode);

    //


    // virtual void query_fund();

};


NS_END;