#pragma once
#include "../ConstantDefs/Marcos.h"
#include "../ConstantDefs/Types.h"
#include "../BaseDefs/BaseObject.hpp"

NS_BEGIN
class ContractInfo;


//委托数据结构
class Entrust : public PoolObject<Entrust>
{
public:
    char          m_strExchg[MAX_EXCHANGE_LENGTH];      //交易所
    char          m_strCode[MAX_INSTRUMENT_LENGTH];     //合约代码
    double        m_dVolume;                            //委托数量
    double        m_dPrice;                             //委托价格
    bool          m_bIsNet;                             //
    bool          m_bIsBuy;                             //是否买入
    DirectionType m_direction;                          //买卖方向
    PriceType     m_priceType;                          //价格类型
    OrderFlag     m_orderFlag;                          //订单标志
    OffsetType    m_offsetType;                         //开平类型
    char          m_strEntrustID[64] = { 0 };           //委托ID
    char          m_strUserTag[64] = { 0 };             //用户自定义标签
    BusinessType  m_businessType;                       //业务类型
    ContractInfo* m_pContract;                          //合约信息
public:
    Entrust()
      : m_dVolume(0)
      , m_dPrice(0)
      , m_bIsNet(false)
      , m_bIsBuy(true)
      , m_direction(DT_Long)
      , m_priceType(PT_AnyPrice)
      , m_orderFlag(OF_Normal)
      , m_offsetType(OT_Open)
      , m_pContract(nullptr)
    {}
    virtual ~Entrust() {}

public:
    static Entrust* create()
    {
        Entrust* pret = Entrust::allocate();
        //TODO初始化
        if (pret)
        {
            return pret;
        }
        return nullptr;

    }
};



//委托操作: 撤单、改单



//订单信息: 订单状态更新



//成交信息: 最新成交



//持仓信息



//账户信息




//结算单信息




NS_END;