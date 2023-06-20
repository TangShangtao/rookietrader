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
    double        m_dVolume;                            //委托数量
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
    , m_orderFlag(OF_Normal)
    , m_priceType(PT_AnyPrice)
    , m_bIsNet(false)
    , m_bIsBuy(true){}
    virtual ~Entrust() {}

public:
    static Entrust* create(const char* code, double vol, double price, const char* exchg = "", BusinessType bType = BT_CASH)
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
	inline void setExchange(const char* exchg, std::size_t len = 0)
	{
		StrUtils::my_strncpy(m_strExchg, exchg, len);
    }
	inline void setCode(const char* code, std::size_t len = 0)
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
    static EntrustAction* create(const char* code, const char* exchg = "", double vol = 0, double price = 0, BusinessType bType = BT_CASH)
    {
        EntrustAction* pret = EntrustAction::allocate();
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


//订单信息: 订单状态更新
class OrderInfo : public PoolObject<OrderInfo>
{
private:
    //以下与Entrust一致//TODO为什么不选择一个指向Entrust的指针

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
    bool          m_bIsError;                           //是否错误订单
    OrderState    m_orderState;                         //订单状态
    OrderType     m_orderType;                          //订单类型
    //订单ID
    char          m_strOrderID[64] = { 0 };             //订单ID
    //其他
    std::string   m_strStateMsg;
public:
	OrderInfo()
		: m_uInsertDate(0)
        , m_uInsertTime(0)
		, m_dVolTraded(0)
		, m_dVolLeft(0)
		, m_bIsError(false)
        , m_orderState(OS_Submitting)
		, m_orderType(OT_Normal)
        {}
	virtual ~OrderInfo(){}
    //根据委托初始化订单
    static OrderInfo* create(Entrust* pEntrust)
    {
        OrderInfo* pret = OrderInfo::allocate();
        if (pret && pEntrust)
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
        return nullptr;
    }
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
	inline void	set_error(bool bError = true){ m_bIsError = bError; }
	inline bool	is_error() const{ return m_bIsError; }


};


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


//结算单信息




NS_END;