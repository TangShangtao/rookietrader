#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/ConstantDefs/Types.h"
#include "Defines/BaseDefs/BaseObject.hpp"
#include "Utils/StrUtils.hpp"
#include "Entrust.hpp"
#include <string.h>
NS_BEGIN

//持仓明细
class PositionItem : public PoolObject<PositionItem>
{
private:
    //目标合约信息
	char			m_strExchg[MAX_EXCHANGE_LENGTH];
	char			m_strCode[MAX_INSTRUMENT_LENGTH];
    ContractInfo*   m_pContract;         //合约信息
	//持仓信息
    DirectionType   m_direction;
	double		    m_dPrePosition;		//昨仓
	double		    m_dNewPosition;		//今仓
	double		    m_dAvailPrePos;		//可平昨仓
	double		    m_dAvailNewPos;		//可平今仓
	double		    m_dTotalPosCost;	//持仓总成本
	double		    m_dMargin;			//占用保证金
	double		    m_dAvgPrice;		//持仓均价
	double		    m_dDynProfit;		//浮动盈亏
    //其他
    BusinessType    m_businessType;     //业务类型
    char			m_strCurrency[8] = { 0 };    //币种
    
public:
	PositionItem()
		: m_pContract(NULL) 
        , m_direction(DT_Long)
		, m_dPrePosition(0)
		, m_dNewPosition(0)
		, m_dAvailPrePos(0)
		, m_dAvailNewPos(0)
		, m_dTotalPosCost(0)
		, m_dMargin(0)
		, m_dAvgPrice(0)
		, m_dDynProfit(0)
		, m_businessType(BT_CASH)
	{}
	virtual ~PositionItem(){}
	static inline PositionItem* create(const char* code, const char* currency = "CNY", const char* exchg = "", BusinessType bType = BT_CASH)
	{
		PositionItem *pRet = PositionItem::allocate();
		StrUtils::my_strncpy(pRet->m_strExchg, exchg);
		StrUtils::my_strncpy(pRet->m_strCode, code);
		StrUtils::my_strncpy(pRet->m_strCurrency, currency);
		pRet->m_businessType = bType;

		return pRet;
	}

    void set_direction(DirectionType dType){m_direction = dType;}
    void set_pre_position(double prePos){ m_dPrePosition = prePos; }
    void set_new_position(double newPos){ m_dNewPosition = newPos; }
    void set_avail_pre_pos(double availPos){ m_dAvailPrePos = availPos; }
    void set_avail_new_pos(double availPos){ m_dAvailNewPos = availPos; }
    void set_position_cost(double cost){m_dTotalPosCost = cost;}
    void set_margin(double margin){ m_dMargin = margin; }
    void set_avg_price(double avgPrice){ m_dAvgPrice = avgPrice; }
    void set_dyn_profit(double profit){ m_dDynProfit = profit; }

    DirectionType get_direction() const{return m_direction;}
    double get_pre_position() const{ return m_dPrePosition; }
    double get_new_position() const{ return m_dNewPosition; }
    double get_avail_pre_pos() const{ return m_dAvailPrePos; }
    double get_avail_new_pos() const{ return m_dAvailNewPos; }

    double get_total_position() const{ return m_dPrePosition + m_dNewPosition; }
    double get_avail_position() const{ return m_dAvailPrePos + m_dAvailNewPos; }

    double get_frozen_position() const{ return get_total_position() - get_avail_position(); }
    double get_frozen_new_pos() const{ return m_dNewPosition - m_dAvailNewPos; }
    double get_frozen_pre_pos() const{ return m_dPrePosition - m_dAvailPrePos; }

    double get_position_cost() const{ return m_dTotalPosCost; }
    double get_margin() const{ return m_dMargin; }
    double get_avg_price() const{ return m_dAvgPrice; }
    double get_dyn_profit() const{ return m_dDynProfit; }

    const char* get_code() const{ return m_strCode; }
    const char* get_currency() const{ return m_strCurrency; }
    const char* get_exchg() const{ return m_strExchg; }

    void set_business_type(BusinessType bType) { m_businessType = bType; }
    BusinessType get_business_type() const { return m_businessType; }

    void set_contract_info(ContractInfo* cInfo) { m_pContract = cInfo; }
    ContractInfo* get_contract_info() const { return m_pContract; }
};

NS_END;