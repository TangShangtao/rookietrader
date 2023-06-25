#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/ConstantDefs/Types.h"
#include "Defines/BaseDefs/BaseObject.hpp"
#include "Utils/StrUtils.hpp"
#include "Entrust.hpp"
#include <string.h>
NS_BEGIN
//成交信息: 最新成交
class TradeInfo : public PoolObject<TradeInfo>
{
private:
    //目标合约信息
    char          m_strExchg[MAX_EXCHANGE_LENGTH];      //交易所
    char          m_strCode[MAX_INSTRUMENT_LENGTH];     //合约代码
    ContractInfo* m_pContract;                          //合约信息
    //成交时间
    uint32_t      m_uTradeDate;                         //成交日期
    uint64_t      m_uTradeTime;                         //成交时间
    //成交信息
    double        m_dVolume;                            //成交数量
    double        m_dPrice;                             //成交价格
    TradeType     m_tradeType;                          //成交类型
    double        m_uAmount;                            //TODO
    //原订单信息
    bool          m_bIsNet;                             //TODO
    bool          m_bIsBuy;                             //是否买入
    DirectionType m_direction;                          //买卖方向
    OffsetType    m_offsetType;                         //开平类型
    OrderType     m_orderType;                          //订单类型
    //其他
    BusinessType  m_businessType;                       //业务类型
    //成交ID、订单ID
    char          m_strTradeID[64] = { 0 };             //成交ID
    char          m_strRefOrder[64] = { 0 };            //本地委托序列号//TODO ?
    //其他
    char          m_strUserTag[64] = { 0 };             //用户自定义标签

public:
	TradeInfo()
		: m_pContract(NULL)
		, m_dPrice(0)
		, m_tradeType(TT_Common)        
		, m_uAmount(0)

        , m_orderType(OT_Normal)
		, m_businessType(BT_CASH)

	{}
	virtual ~TradeInfo(){}
	static inline TradeInfo* create(const char* code, const char* exchg = "", BusinessType bType = BT_CASH)
	{
		TradeInfo *pRet = TradeInfo::allocate();
		StrUtils::my_strncpy(pRet->m_strExchg, exchg);
		StrUtils::my_strncpy(pRet->m_strCode, code);
		pRet->m_businessType = bType;

		return pRet;
	}

	inline void set_tradeID(const char* tradeid) {StrUtils::my_strncpy(m_strTradeID, tradeid); }
	inline void set_ref_order(const char* oid) {StrUtils::my_strncpy(m_strRefOrder, oid); }
	
	inline void set_direction(DirectionType dType){m_direction = dType;}
	inline void set_offset_type(OffsetType oType){m_offsetType = oType;}
	inline void set_order_type(OrderType ot){m_orderType = ot;}
	inline void set_trade_type(TradeType tt){m_tradeType = tt;}
	inline void set_trade_date(uint32_t uDate){m_uTradeDate = uDate;}
	inline void set_trade_time(uint64_t uTime){m_uTradeTime = uTime;}
	inline void set_volume(double volume){m_dVolume = volume;}
	inline void set_price(double price){ m_dPrice = price; }

	inline void set_amount(double amount){ m_uAmount = amount; }

	inline DirectionType	get_direction() const{return m_direction;}
	inline OffsetType	get_offset_type() const{return m_offsetType;}
	inline OrderType		get_order_type() const{return m_orderType;}
	inline TradeType		get_trade_type() const{return m_tradeType;}

	inline double get_volume() const{ return m_dVolume; }
	inline double get_price() const{ return m_dPrice; }

	inline const char*	get_code() const { return m_strCode; }
	inline const char*	get_exchg() const { return m_strExchg; }
	inline const char*	get_tradeID() const { return m_strTradeID; }
	inline const char*	get_ref_order() const { return m_strRefOrder; }

	inline char*	get_tradeID() { return m_strTradeID; }
	inline char*	get_ref_order() { return m_strRefOrder; }

	inline uint32_t get_trade_date() const{return m_uTradeDate;}
	inline uint64_t get_trade_time() const{return m_uTradeTime;}

	inline double get_amount() const{ return m_uAmount;}

	inline void set_user_tag(const char* tag) {StrUtils::my_strncpy(m_strUserTag, tag); }
	inline const char* get_user_tag() const { return m_strUserTag; }

	inline void set_businessType(BusinessType bType) {m_businessType = bType; }
	inline BusinessType	get_business_type() const { return m_businessType; }

	inline void set_net_direction(bool isBuy) { m_bIsNet = true; m_bIsBuy = isBuy; }
	inline bool is_net() const { return m_bIsNet; }
	inline bool is_buy() const { return m_bIsBuy; }

	inline void set_contract_info(ContractInfo* cInfo) { m_pContract = cInfo; }
	inline ContractInfo* get_contract_info() const { return m_pContract; }

};

NS_END;