#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/ConstantDefs/Types.h"
#include <stdint.h>

NS_BEGIN
class Variant;
class Entrust;
class OrderInfo;
class TradeInfo;
class EntrustAction;
class AccountInfo;
class PositionItem;
class ContractInfo;
class Error;
class TickData;
class Notify;
class Array;

//交易网关对外提供的交易接口, 主要提供给GatewayAdapter
class IGatewayApi
{

public:
    virtual ~IGatewayApi(){}
public:
    
    
    //设置接口//
    
    //初始化接口实例
    virtual int init(Variant* cfg){return -1;}
    //释放接口实例
    virtual int release(){return -1;}
    //注册回调接口
    // virtual void register_spi(IGatewaySpi* listener){}
    
    
    //功能接口//

    //与柜台连接
    virtual int connect(){return -1;}
    //与柜台断开连接
    virtual int disconnect(){return -1;}
    //是否连接成功
    virtual bool is_connected(){return false;}
    //登录
    virtual int login(){return -1;}
    //登出
    virtual int logout(){return -1;}
    //确认结算单
    virtual int confirm(){return -1;}
    //下达委托
    virtual int order_insert(Entrust* entrust){return -1;}
    //
    virtual int order_action(EntrustAction* action){return -1;}
    //查询账户
    virtual int query_account(){return -1;}
    //查询持仓
    virtual int query_positions(){return -1;}
    //查询历史订单
    virtual int query_orders(){return -1;}
    //查询历史成交
    virtual int query_trades(){return -1;}
    //查询指定日期的结算单
    virtual int query_settlement(uint32_t uDate){return -1;}
};

// class IGatewaySpi
// {
// public:
//     // virtual CommonMgr* get_common_mgr() = 0;
// //交易网关通知时调用的回调接口, 由GatewayAdapter继承
// public:
    
//     //通知连接结果
//     virtual void on_connect(bool bSucc) = 0;
//     //通知登录结果
//     virtual void on_login(bool bSucc, const char* msg, uint32_t tradingdate) = 0;
//     //通知登出结果
//     virtual void on_logout(){}
//     //通知下达委托结果
//     virtual void on_rsp_order_insert(Entrust* entrust, Error* err){}
//     //通知资金查询结果
//     virtual void on_rtn_query_account(Array* ayAccounts){}
//     //通知持仓查询结果
//     virtual void on_rtn_query_positions(const Array* ayPositions){}
//     //通知订单查询结果
//     virtual void on_rtn_query_orders(const Array* ayOrders){}
//     //通知成交查询结果
//     virtual void on_rtn_query_trades(const Array* ayTrades){}
//     //通知结算单查询结果
//     virtual void on_rtn_settlement_info(uint32_t uDate, const char* content){}
//     //通知订单状态更新
//     virtual void on_rtn_order(OrderInfo* orderInfo){}
//     //通知成交
//     virtual void on_rtn_trade(TradeInfo* tradeInfo){}
//     //通知交易网关错误
//     virtual void on_rsp_gateway_error(Error* err){}
//     //通知合约交易状态更新
//     virtual void on_rtn_instrument_status(const char* exchg, const char* code, TradeStatus state){}
// };

NS_END;
