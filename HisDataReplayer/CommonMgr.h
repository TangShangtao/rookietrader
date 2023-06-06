//通用配置信息管理器
#pragma once
#include "../Defines/ConstantDefs/Marcos.h"
#include "../Defines/Containers/HashMap.hpp"
#include "../Defines/Containers/Array.hpp"

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <set>

NS_BEGIN

class CommodityInfo;
class SessionInfo;
class ContractInfo;
class Array;

class CommonMgr
{
public:
    typedef std::unordered_set<uint32_t> HolidaySet;
    typedef struct _TradingDayTpl
    {
        uint32_t cur_tdate;
        HolidaySet holidays;
        _TradingDayTpl() : cur_tdate(0) {}
    }TradingDayTpl; 
    typedef std::unordered_map<std::string, TradingDayTpl> TradingDayTplMap;
    typedef HashMap<std::string> ContractList;
    typedef HashMap<std::string> ExchgContract;
    typedef HashMap<std::string> ContractMap;
    typedef HashMap<std::string> SessionMap;
    typedef HashMap<std::string> CommodityMap;
    typedef std::set<std::string> CodeSet;
    typedef std::unordered_map<std::string, CodeSet> SessionCodeMap;

public:
    TradingDayTplMap m_tradingDayTplMap;
    SessionCodeMap m_sessionCodeMap;
    ExchgContract* m_exchgContractMap;
    SessionMap* m_sessionMap;
    CommodityMap* m_commodityMap;
    ContractMap* m_contractMap;

public:
    
    bool load_commodities(const char* filename);
    bool load_contracts(const char* filename);
    bool load_sessions(const char* filename);
    bool load_holidays(const char* filename);

    CommodityInfo*  get_commodity(const char* ExchgPid);
    CommodityInfo*  get_commodity(const char* exchg, const char* pid);
    ContractInfo*   get_contract(const char* exchg, const char* code);
    ContractInfo*   get_contract(const char* ExchgCode);
    Array*          get_contracts(const char* exchg = "");
    SessionInfo*    get_session(const char* Sid);
    SessionInfo*    get_session(const char* code, const char* exchg = "");
    Array*          get_all_sessions();

	bool			is_holiday(const char* ExchgPid, uint32_t uDate, bool isTpl = false);

	uint32_t		calc_trading_date(const char* ExchgPid, uint32_t uDate, uint32_t uTime, bool is_session = false);
	uint64_t		get_boundary_time(const char* ExchgPid, uint32_t tDate, bool is_session = false, bool is_start = true);


public:
	uint32_t	getTradingDate(const char* stdPID, uint32_t uOffDate = 0, uint32_t uOffMinute = 0, bool isTpl = false);
	uint32_t	getNextTDate(const char* stdPID, uint32_t uDate, int days = 1, bool isTpl = false);
	uint32_t	getPrevTDate(const char* stdPID, uint32_t uDate, int days = 1, bool isTpl = false);
	bool		isTradingDate(const char* stdPID, uint32_t uDate, bool isTpl = false);
	void		setTradingDate(const char* stdPID, uint32_t uDate, bool isTpl = false);

	CodeSet*	getSessionComms(const char* sid);

    void		release();

private:
	const char* getTplIDByPID(const char* stdPID);



};


NS_END;