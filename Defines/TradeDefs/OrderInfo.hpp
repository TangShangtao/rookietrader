#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/ConstantDefs/Types.h"
#include "Defines/BaseDefs/BaseObject.hpp"
#include "Utils/StrUtils.hpp"
#include "Entrust.hpp"
#include <string.h>
NS_BEGIN


class ContractInfo;

//订单信息: 订单状态更新
class OrderInfo : public PoolObject<OrderInfo>
{
private:
    //以下与Entrust一致//TODO为什么不选择一个指向Entrust的指针

    //目标合约信息
    char          m_strExchg[MAX_EXCHANGE_LENGTH];      //交易所
    char          m_strCode[MAX_INSTRUMENT_LENGTH];     //合约代码
    ContractInfo* m_pContract;                          //合约信息
    //下为委托信息
    DirectionType m_direction;                          //买卖方向
    OffsetType    m_offsetType;                         //开平类型
    OrderFlag     m_orderFlag;                          //订单标志
    PriceType     m_priceType;                          //价格类型
    double        m_dPrice;                             //委托价格
    double        m_dVolume;                            //委托数量
    bool          m_bIsNet;                             //
    bool          m_bIsBuy;                             //是否买入
    //委托ID
    char          m_strEntrustID[64] = { 0 };           //委托ID
    //其他
    char          m_strUserTag[64] = { 0 };             //用户自定义标签
    BusinessType  m_businessType;                       //业务类型
    
    //以下为Order新增

    //挂单时间
    uint32_t      m_uInsertDate;                        //挂单日期
    uint64_t      m_uInsertTime;                        //挂单时间
    //挂单信息
    double        m_dVolTraded;                         //已成交数量
    double        m_dVolLeft;                           //剩余数量
    bool          m_bIsRejected;                        //是否被拒绝
    OrderState    m_orderState;                         //订单状态
    OrderType     m_orderType;                          //订单类型
    //订单ID
    char          m_strOrderID[64] = { 0 };             //订单ID, 由CTP生成
    //其他
    std::string   m_strStateMsg;
public:
	OrderInfo()
		: m_uInsertDate(0)
        , m_uInsertTime(0)
		, m_dVolTraded(0)
		, m_dVolLeft(0)
		, m_bIsRejected(false)
        , m_orderState(OS_Submitting)
		, m_orderType(OT_Normal)
        {}
	virtual ~OrderInfo(){}
    //根据委托初始化订单
    static OrderInfo* create(Entrust* pEntrust = nullptr)
    {
        OrderInfo* pret = OrderInfo::allocate();
		if (pret == nullptr) return nullptr;
        if (pEntrust != nullptr)
        {
            StrUtils::my_strncpy(pret->m_strExchg, pEntrust->get_exchg());
            StrUtils::my_strncpy(pret->m_strCode, pEntrust->get_code());
            pret->m_dVolume = pEntrust->get_volume();
            pret->m_dPrice = pEntrust->get_price();
            pret->m_direction = pEntrust->get_direction();
            pret->m_priceType = pEntrust->get_price_type();
            pret->m_orderFlag = pEntrust->get_order_flag();
            pret->m_offsetType = pEntrust->get_offset_type();
            StrUtils::my_strncpy(pret->m_strEntrustID, pEntrust->get_entrustID());
            StrUtils::my_strncpy(pret->m_strUserTag, pEntrust->get_user_tag());
            pret->m_businessType = pEntrust->get_business_type();
            pret->m_pContract = pEntrust->get_contract_info();
            pret->m_bIsNet = pEntrust->is_net();
            pret->m_bIsBuy = pEntrust->is_buy();
            
            return pret;
        }
		// else if (pret != nullptr) return pret;
		// else return nullptr;
		else return pret;
    }
//Entrust一致
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
//Order新增
public:
	inline void	set_order_date(uint32_t uDate){m_uInsertDate = uDate;}
	inline void	set_order_time(uint64_t uTime){m_uInsertTime = uTime;}
	inline void	set_vol_traded(double vol){ m_dVolTraded = vol; }
	inline void	set_vol_left(double vol){ m_dVolLeft = vol; }
	
	inline void	set_orderID(const char* oid) { StrUtils::my_strncpy(m_strOrderID, oid); }
	inline void	set_order_state(OrderState os){m_orderState = os;}
	inline void	set_order_type(OrderType ot){m_orderType = ot;}

	inline uint32_t get_order_date() const{return m_uInsertDate;}
	inline uint64_t get_order_time() const{return m_uInsertTime;}
	inline double get_vol_traded() const{ return m_dVolTraded; }
	inline double get_vol_left() const{ return m_dVolLeft; }
    
	inline OrderState		get_order_state() const { return m_orderState; }
	inline OrderType			get_order_type() const { return m_orderType; }
	inline const char*			get_orderID() const { return m_strOrderID; }
	inline char*			get_orderID() { return m_strOrderID; }

	inline void	set_state_msg(const char* msg){m_strStateMsg = msg;}
	inline const char* get_state_msg() const{return m_strStateMsg.c_str();}

	inline bool	is_alive() const
	{
		switch(m_orderState)
		{
		case OS_AllTraded:
		case OS_Canceled:
			return false;
		default:
			return true;
		}
	}
	inline void	set_rejected(bool bRejected = true){m_bIsRejected = bRejected;}
	inline bool	is_rejected() const{return m_bIsRejected;}


};

NS_END;