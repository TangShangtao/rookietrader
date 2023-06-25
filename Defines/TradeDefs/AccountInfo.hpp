#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/ConstantDefs/Types.h"
#include "Defines/BaseDefs/BaseObject.hpp"
#include "Utils/StrUtils.hpp"
#include "Entrust.hpp"
#include <string.h>
NS_BEGIN
//账户信息
class AccountInfo : public PoolObject<AccountInfo>
{
private:
    std::string m_strCurrency;      //币种
    double      m_dBalance;         //账户余额
    double      m_dPreBalance;      //上日账户余额
    double      m_uMargin;          //保证金
    double      m_dFrozenMargin;    //冻结保证金
    double      m_dCommission;      //手续费
    double      m_dFrozenCommission;//冻结手续费
    double      m_dDynProfit;       //浮动盈亏
    double      m_dCloseProfit;     //平仓盈亏
    double      m_dDeposit;         //入金
    double      m_dWithdraw;        //出金
    double      m_dAvailable;       //可用资金
public:
    static inline AccountInfo* create(){return AccountInfo::allocate();}

    inline void set_currency(const char* currency){ m_strCurrency = currency; }

    inline void set_balance(double balance){m_dBalance = balance;}
    inline void set_pre_balance(double prebalance){m_dPreBalance = prebalance;}
    inline void set_margin(double margin){m_uMargin = margin;}
    inline void set_frozen_margin(double frozenmargin){m_dFrozenMargin = frozenmargin;}
    inline void set_close_profit(double closeprofit){m_dCloseProfit = closeprofit;}
    inline void set_dyn_profit(double dynprofit){m_dDynProfit = dynprofit;}
    inline void set_deposit(double deposit){m_dDeposit = deposit;}
    inline void set_withdraw(double withdraw){m_dWithdraw = withdraw;}
    inline void set_commission(double commission){m_dCommission = commission;}
    inline void set_frozen_commission(double frozencommission){m_dFrozenCommission = frozencommission;}
    inline void set_available(double available){m_dAvailable = available;}

    inline double get_balance() const{return m_dBalance;}
    inline double get_pre_balance() const{return m_dPreBalance;}
    inline double get_margin() const{return m_uMargin;}
    inline double get_frozen_margin() const{return m_dFrozenMargin;}
    inline double get_close_profit() const{return m_dCloseProfit;}
    inline double get_dyn_profit() const{return m_dDynProfit;}
    inline double get_deposit() const{return m_dDeposit;}
    inline double get_withdraw() const{return m_dWithdraw;}
    inline double get_commission() const{return m_dCommission;}
    inline double get_frozen_commission() const{return m_dFrozenCommission;}
    inline double get_available() const{return m_dAvailable;}

    inline const char* get_currency() const{ return m_strCurrency.c_str(); }

};



NS_END;