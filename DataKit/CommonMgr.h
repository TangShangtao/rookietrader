//通用配置信息管理器
//1.品种信息
//2.合约信息、费用信息
//3.节假日信息
//4.交易时间段信息
//5.主力合约切换信息
#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/Containers/HashMap.hpp"
#include "Defines/Containers/Array.hpp"
#include "Defines/Containers/Variant.hpp"
#include "Defines/CommonDefs/CommodityInfo.hpp"
#include "Defines/CommonDefs/ContractInfo.hpp"
#include "Defines/CommonDefs/SessionInfo.hpp"

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <set>


USING_NS;

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
    typedef std::unordered_map<std::string, TradingDayTpl> TradingDayTplMap; //{"CHINA" : TradingDayTpl}
    typedef HashMap<std::string> ContractList;  //{"rb2312" : ContractInfo*}, 记录某合约的合约信息
    typedef HashMap<std::string> ExchgContract;
    typedef HashMap<std::string> ContractMap;
    typedef HashMap<std::string> SessionMap;
    typedef HashMap<std::string> CommodityMap;
    typedef std::set<std::string> CodeSet;      //(SHFE.rb)
    typedef std::unordered_map<std::string, CodeSet> SessionCodeMap;

public:
    TradingDayTplMap m_mapTradingDayTpl;    //{"CHINA" : TradingDayTpl}
    SessionCodeMap m_mapSessionCode;        //{"FD0900" : CodeSet}, 记录属于某交易时间段类型的所有品种
    ExchgContract* m_mapExchgContract;      //{"SHFE" : ContractList*}, 记录属于某交易所的所有合约以及合约信息
    SessionMap* m_mapSession;               //{"FD0900" : SessionInfo*}, 记录某交易时间段类型的交易时间段信息
    CommodityMap* m_mapCommodity;           //{SHFE.rb : CommodityInfo*}, 记录某品种的品种信息
    ContractMap* m_mapContract;             //{"rb2312" : [ContractInfo*]}, 记录某合约的合约信息(为什么以Array的形式)

public:
    CommonMgr();
    ~CommonMgr();
public:
    void		release();

public:
    //需按顺序读取信息
    bool load_sessions(const char* filename);
    bool load_commodities(const char* filename);    //commodities依赖sessions
    bool load_contracts(const char* filename);      //contracts依赖commodities
    bool load_holidays(const char* filename);

    CommodityInfo*  get_commodity(const char* ExchgPid);
    CommodityInfo*  get_commodity(const char* exchg, const char* pid);
    ContractInfo*   get_contract(const char* code, const char* exchg); //选择输入其一进行查询
    Array*          get_contracts(const char* exchg = "");
    SessionInfo*    get_session(const char* sid);
    SessionInfo*    get_session_by_code(const char* code, const char* exchg = "");
    Array*          get_all_sessions();
	
	uint32_t		calc_trading_date(const char* ExchgPid, uint32_t uDate, uint32_t uTime, bool isSession = false);
	uint64_t		get_boundary_time(const char* ExchgPid, uint32_t tDate, bool is_session = false, bool isStart = true);

    
public:
    bool		is_holiday(const char* ExchgPid, uint32_t uDate, bool isTpl = false);//isTpl:ExchgPid处输入的是否为交易日模板id("CHINA")
	uint32_t	get_trading_date(const char* ExchgPid, uint32_t uOffDate = 0, bool isTpl = false);
	uint32_t	get_next_tdate(const char* ExchgPid, uint32_t uDate, int days = 1, bool isTpl = false);
	uint32_t	get_prev_tdate(const char* ExchgPid, uint32_t uDate, int days = 1, bool isTpl = false);
	bool		is_trading_date(const char* ExchgPid, uint32_t uDate, bool isTpl = false);
	void		set_trading_date(const char* ExchgPid, uint32_t uDate, bool isTpl = false);
	CodeSet*	get_session_comms(const char* sid);


private:
	const char* get_tplid(const char* ExchgPid);
    void set_session(SessionInfo* sessionInfo, Variant* field);
    void set_commodity(CommodityInfo* commodityInfo, Variant* field);
    void set_contract(ContractInfo* contractInfo, Variant* field);

        //
    void debug();



};


