// 记录品种信息, 来源于配置文件
// commodity和product是一个意思，例如IF、RB
#pragma once
#include "../ConstantDefs/Marcos.h"
#include "../BaseDefs/BaseObject.hpp"
#include "../ConstantDefs/Types.h"

#include <set>
#include <string>
#include <sstream>

NS_BEGIN
class SessionInfo;

class CommodityInfo : public BaseObject
{
public:
    typedef std::set<std::string> CodeSet;
public:
	std::string	m_strName;		//螺纹
	std::string	m_strExchg;		//SHFE
	std::string	m_strPid;		//rb
	std::string m_strSession;	//FD0900	(期货白盘)
	std::string m_strTrdTpl;	//CHINA		(trade template)
	std::string m_strCurrency;	//CNY
	std::string m_strExchgPid;	//SHFE.rb

	uint32_t	m_uVolScale;	//合约乘数
	double		m_dPriceTick;	//最小价格变动单位
	//uint32_t	m_uPrecision;	//价格精度
	double		m_dLotTick;		//数量精度
	double		m_dMinLots;		//最小交易数量

	ContractCategory	m_ccCategory;	//品种分类，期货、股票、期权等
	CoverMode			m_coverMode;	//平仓类型
	PriceMode			m_priceMode;	//价格类型
	TradingMode			m_tradeMode;	//交易类型

	CodeSet				m_setCodes;     //合约代码集合

	SessionInfo*		m_pSession;     //交易时间段
public:
    static CommodityInfo* create(const char* name, const char* exchg, const char* pid, const char* session, const char*trdtpl, const char* currency = "CNY")
    {
        CommodityInfo* pret = new CommodityInfo;
		pret->m_strName = name;
		pret->m_strExchg = exchg;
		pret->m_strPid = pid;
		pret->m_strSession = session;
		pret->m_strTrdTpl = trdtpl;
		pret->m_strCurrency = currency;
		std::stringstream ss;
		ss << exchg << "." << pid;
		pret->m_strExchgPid = ss.str();
		return pret;
    }
	void set_vol_scale(uint32_t volScale){ m_uVolScale = volScale; }
	void set_price_tick(double pxTick){ m_dPriceTick = pxTick; }
	void set_lots_tick(double lotsTick){ m_dLotTick = lotsTick; }
	void set_min_lots(double minLots) { m_dMinLots = minLots; }
	void set_category(ContractCategory cat){ m_ccCategory = cat; }
	void set_cover_mode(CoverMode cm){ m_coverMode = cm; }
	void set_price_mode(PriceMode pm){ m_priceMode = pm; }
	void set_trading_mode(TradingMode tm) { m_tradeMode = tm; }

	const char* get_name() const { return m_strName.c_str(); }
	const char* get_exchg() const { return m_strExchg.c_str(); }
	const char* get_product() const { return m_strPid.c_str(); }
	const char* get_session() const { return m_strSession.c_str(); }
	const char* get_trdtpl() const { return m_strTrdTpl.c_str(); }
	const char* get_currency() const { return m_strCurrency.c_str(); }
	const char* get_full_pid() const { return m_strExchgPid.c_str(); }

	uint32_t get_vol_scale() const { return m_uVolScale; }
	double get_price_tick() const { return m_dPriceTick; }
	double get_lots_tick() const { return m_dLotTick; }
	double get_min_lots() const { return m_dMinLots; }
	ContractCategory get_category() const { return m_ccCategory; }
	CoverMode get_cover_mode() const { return m_coverMode; }
	PriceMode get_price_mode() const { return m_priceMode; }
	TradingMode get_trading_mode() const { return m_tradeMode; }

	void add_code(const char* code){ m_setCodes.insert(code); }
	const CodeSet& get_codes() const { return m_setCodes; }
	void set_session_info(SessionInfo* session){ m_pSession = session; }
	SessionInfo* get_session_info() const { return m_pSession; }

	bool is_future() const { return m_ccCategory == CC_Future; }
	bool is_stock() const { return m_ccCategory == CC_Stock; }
	bool is_option() const { return m_ccCategory == CC_FutOption || m_ccCategory == CC_SpotOption || m_ccCategory == CC_ETFOption; }
};

//合约: IF1706


NS_END;