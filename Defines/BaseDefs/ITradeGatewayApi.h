#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/ConstantDefs/Types.h"
#include "DataKit/CommonMgr.h"
#include <stdint.h>

NS_BEGIN
class Variant;
class Entrust;
class EntrustAction;
class OrderInfo;
class TradeInfo;



//交易网关对外提供的交易接口, 主要提供给GatewayAdapter
class ITradeGatewayApi
{
public:
    virtual ~ITradeGatewayApi() {}
public:
    
    virtual int init(Variant* cfg){return -1;}
    virtual int release(){return -1;}
    // virtual void register_sink(ITradeGatewaySink* listener){}
    
    virtual int connect(){return -1;}
    virtual int disconnect(){return -1;}
    virtual bool is_connected(){return false;}
    virtual int login(){return -1;}
    virtual int logout(){return -1;}
    virtual int confirm(){return -1;}

    virtual bool generate_entrustID(char* buffer, int length){return false;} 
    virtual int order_insert(Entrust* entrust){return -1;}
    virtual int order_action(EntrustAction* action){return -1;}
    
    virtual int query_account(){return -1;}
    virtual int query_positions(){return -1;}
    virtual int query_orders(){return -1;}
    virtual int query_trades(){return -1;}
    virtual int query_settlement(uint32_t uDate){return -1;}
};

NS_END;
