#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/ConstantDefs/Types.h"
#include "Defines/BaseDefs/BaseObject.hpp"
#include "Utils/StrUtils.hpp"
#include <string.h>
NS_BEGIN

class ContractInfo;


//委托数据结构
class Entrust : public PoolObject<Entrust>
{
private:
    //目标合约信息
    char          m_strExchg[MAX_EXCHANGE_LENGTH];      //交易所
    char          m_strCode[MAX_INSTRUMENT_LENGTH];     //合约代码
    ContractInfo* m_pContract;                          //合约信息
    //下达委托信息
    DirectionType m_direction;                          //买卖方向
    OffsetType    m_offsetType;                         //开平类型
    OrderFlag     m_orderFlag;                          //订单标志
    PriceType     m_priceType;                          //价格类型
    double        m_dPrice;                             //委托价格
    int           m_dVolume;                            //委托数量
    bool          m_bIsNet;                             //
    bool          m_bIsBuy;                             //是否买入
    //委托ID
    char          m_strEntrustID[64] = { 0 };           //委托ID
    //其他
    char          m_strUserTag[64] = { 0 };             //用户自定义标签
    BusinessType  m_businessType;                       //业务类型
    
public:
    Entrust()
    : m_pContract(nullptr)
    , m_direction(DT_Long)
    , m_offsetType(OT_Open)
    , m_orderFlag(OF_NOR)
    , m_priceType(PT_AnyPrice)
    , m_bIsNet(false)
    , m_bIsBuy(true){}
    virtual ~Entrust() {}

public:
    static Entrust* create(const char* code, int vol, double price, const char* exchg, BusinessType bType = BT_CASH)
    {
        Entrust* pret = Entrust::allocate();
        //TODO初始化
        if (pret)
        {
            StrUtils::my_strncpy(pret->m_strExchg, exchg);
            StrUtils::my_strncpy(pret->m_strCode, code);
            pret->m_dVolume = vol;
            pret->m_dPrice = price;
            pret->m_businessType = bType;
            return pret;
        }
        return nullptr;

    }
	inline void set_exchg(const char* exchg, std::size_t len = 0)
	{
		StrUtils::my_strncpy(m_strExchg, exchg, len);
    }
	inline void set_code(const char* code, std::size_t len = 0)
	{
		StrUtils::my_strncpy(m_strCode, code, len);
    }

	inline void set_direction(DirectionType dType){m_direction = dType;}
	inline void set_price_type(PriceType pType){m_priceType = pType;}
	inline void set_order_flag(OrderFlag oFlag){m_orderFlag = oFlag;}
	inline void set_offset_type(OffsetType oType){m_offsetType = oType;}

	inline DirectionType	get_direction() const{return m_direction;}
	inline PriceType		get_price_type() const{return m_priceType;}
	inline OrderFlag		get_order_flag() const{return m_orderFlag;}
	inline OffsetType	get_offset_type() const{return m_offsetType;}

	inline void set_business_type(BusinessType bType) { m_businessType = bType; }
	inline BusinessType	get_business_type() const { return m_businessType; }

	inline void set_volume(double volume){ m_dVolume = volume; }
	inline void set_price(double price){ m_dPrice = price; }

	inline double get_volume() const{ return m_dVolume; }
	inline double get_price() const{ return m_dPrice; }

	inline const char* get_code() const { return m_strCode; }
	inline const char* get_exchg() const { return m_strExchg; }

	inline void set_entrustID(const char* eid) { StrUtils::my_strncpy(m_strEntrustID, eid); }
	inline const char* get_entrustID() const { return m_strEntrustID; }
	inline char* get_entrustID() { return m_strEntrustID; }

	inline void set_user_tag(const char* tag) { StrUtils::my_strncpy(m_strUserTag, tag); }
	inline const char* get_user_tag() const { return m_strUserTag; }
	inline char* get_user_tag() { return m_strUserTag; }

	inline void set_net_direction(bool isBuy) { m_bIsNet = true; m_bIsBuy = isBuy; }
	inline bool is_net() const { return m_bIsNet; }
	inline bool is_buy() const { return m_bIsBuy; }

	inline void set_contract_info(ContractInfo* cInfo) { m_pContract = cInfo; }
	inline ContractInfo* get_contract_info() const { return m_pContract; }

};

NS_END;