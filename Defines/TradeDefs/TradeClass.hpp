#pragma once
#include "../ConstantDefs/Marcos.h"
#include "../ConstantDefs/Types.h"
#include "../BaseDefs/BaseObject.hpp"

NS_BEGIN

//委托数据结构
class Entrust : public PoolObject<Entrust>
{
public:
    char    m_strExchg[MAX_EXCHANGE_LENGTH];      //交易所
    char    m_strCode[MAX_INSTRUMENT_LENGTH];     //合约代码
    double  m_dVolume;                            //委托数量
    double  m_dPrice;                             //委托价格
    bool    m_bIsNet;                             //是否净委托
    bool    m_bIsBuy;                             //是否买入
    
};



//委托操作: 撤单、改单



//订单信息: 订单状态更新



//成交信息: 最新成交



//持仓信息



//账户信息




//结算单信息




NS_END;