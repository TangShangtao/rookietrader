#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/ConstantDefs/Types.h"
#include "Defines/BaseDefs/BaseObject.hpp"
#include "Utils/StrUtils.hpp"
#include <string.h>
NS_BEGIN

//委托操作: 撤单、改单
class EntrustAction : public PoolObject<EntrustAction>
{
private:
    //目标合约信息
	char			m_strExchg[MAX_EXCHANGE_LENGTH];
	char			m_strCode[MAX_INSTRUMENT_LENGTH];
    //委托操作信息
	double			m_dVolume;
	double			m_dPrice;
	ActionFlag	    m_actionFlag;
    //委托ID和订单ID
	char			m_strEnturstID[64] = { 0 };
	char			m_strOrderID[64] = { 0 };
	//其他
    BusinessType	m_businessType;    

public:
    EntrustAction()
      : m_dVolume(0)
      , m_dPrice(0)
      , m_actionFlag(AF_Cancel)
      , m_businessType(BT_CASH)
    {}
    virtual ~EntrustAction() {}

public:
    static EntrustAction* create(const char* code, const char* exchg = "", double vol = 0, double price = 0)
    {
        EntrustAction* pret = EntrustAction::allocate();
        if (pret)
        {
            StrUtils::my_strncpy(pret->m_strExchg, exchg);
            StrUtils::my_strncpy(pret->m_strCode, code);
            pret->m_dVolume = vol;
            pret->m_dPrice = price;
            return pret;
        }
        return nullptr;
    }
    static EntrustAction* create_cancel_action(const char* eid, const char* oid)
    {
        //TODO wtf?
        // EntrustAction* pret = EntrustAction::allocate();
        EntrustAction* pret = new EntrustAction;
        if (pret)
        {
            StrUtils::my_strncpy(pret->m_strEnturstID, eid);
            StrUtils::my_strncpy(pret->m_strOrderID, oid);
            pret->m_actionFlag = AF_Cancel;
            return pret;
        }
        return nullptr;
    }
public:
	inline void set_volume(double volume){ m_dVolume = volume; }
	inline void set_price(double price){ m_dPrice = price; }

	inline double get_volume() const{ return m_dVolume; }
	inline double get_price() const{ return m_dPrice; }

	inline const char* get_exchg() const { return m_strExchg; }
	inline const char* get_code() const{return m_strCode;}

	inline void set_exchange(const char* exchg) { StrUtils::my_strncpy(m_strExchg, exchg);}
	inline void set_code(const char* code) { StrUtils::my_strncpy(m_strCode, code);}

	inline void set_action_flag(ActionFlag af){m_actionFlag = af;}
	inline ActionFlag get_action_flag() const{return m_actionFlag;}

	inline void set_entrustID(const char* eid) { StrUtils::my_strncpy(m_strEnturstID, eid); }
	inline const char* get_entrustID() const{return m_strEnturstID;}

	inline void set_orderID(const char* oid) { StrUtils::my_strncpy(m_strOrderID, oid); }
	inline const char* get_orderID() const{return m_strOrderID;}

	inline void set_business_type(BusinessType bType) { m_businessType = bType; }
	inline BusinessType	get_business_type() const { return m_businessType; }
};

NS_END;