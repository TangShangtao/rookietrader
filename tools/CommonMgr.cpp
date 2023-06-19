#include "CommonMgr.h"



#include "../DataKit/CfgLoader.h"
#include "../Utils/FileUtils.hpp"
#include "../Utils/StrUtils.hpp"
#include "../Utils/TimeUtils.hpp"
#include "Logger.h"
CommonMgr::CommonMgr()
//初始化列表?
{
    m_mapExchgContract = ExchgContract::create();
    m_mapSession = SessionMap::create();
    m_mapCommodity = CommodityMap::create();
    m_mapContract = ContractMap::create();
}
void CommonMgr::debug()
{
    for (auto it = m_mapExchgContract->begin(); it != m_mapExchgContract->end(); it++)
    {
        
    }
}

CommonMgr::~CommonMgr()
{
    if (m_mapExchgContract)
    {
        debug();
        m_mapExchgContract->release();
        m_mapExchgContract = nullptr;
    }
    if (m_mapSession)
    {
        m_mapSession->release();
        m_mapSession = nullptr;
    }
    if (m_mapCommodity)
    {
        m_mapCommodity->release();
        m_mapCommodity = nullptr;
    }
    if (m_mapContract)
    {
        m_mapContract->release();
        m_mapContract = nullptr;
    }
}
//为什么同时有release和析构函数
void CommonMgr::release()
{
    if (m_mapExchgContract)
    {
        
        m_mapExchgContract->release();
        m_mapExchgContract = nullptr;
    }
    if (m_mapSession)
    {
        m_mapSession->release();
        m_mapSession = nullptr;
    }
    if (m_mapCommodity)
    {
        m_mapCommodity->release();
        m_mapCommodity = nullptr;
    }
    //为什么不释放contractMap
}

bool CommonMgr::load_sessions(const char* filename)
{
    if (!FileUtils::exists(filename))
    {
        Logger::error("Session configuration file {} not exists", filename);
        return false;
    }
    Variant* root = CfgLoader::load_from_file(filename);
    if (!root)
    {
        Logger::error("Failed to load session configuration file {}", filename);
        return false;
    }
    for (const std::string& sid : root->get_keys())
    {
        Variant* field = root->get(sid);
        const char* name = field->get_cstring("name");
        int32_t offset = field->get_int32("offset");
        SessionInfo* sessionInfo = SessionInfo::create(sid.c_str(), name, offset);
        set_session(sessionInfo, field);
        m_mapSession->add(sid, sessionInfo, false);
        //field->release();
    }
    Logger::info("Session configuration file {} loaded", filename);
    root->release();
    return true;
}
bool CommonMgr::load_commodities(const char* filename)
{
    if (!FileUtils::exists(filename))
    {
        Logger::error("Commodities configuration file {} not exists", filename);
        return false;
    }
    Variant* root = CfgLoader::load_from_file(filename);
    if (!root)
    {
        Logger::error("Failed to load commodities configuration file {}", filename);
        return false;
    }
    for (const std::string& exchg : root->get_keys())
    {
        Variant* pidSubv = root->get(exchg);
        for (const std::string& pid : pidSubv->get_keys())
        {
            Variant* field = pidSubv->get(pid);
            const char* name = field->get_cstring("name");
            const char* exchg = field->get_cstring("exchg");
            const char* sid = field->get_cstring("session");
            const char* hid = field->get_cstring("holiday");
            CommodityInfo* commodityInfo = CommodityInfo::create(name, exchg, pid.c_str(), sid, hid);
            set_commodity(commodityInfo, field);
            std::string ExchgPid = StrUtils::format("{}.{}", exchg, pid.c_str());
            m_mapCommodity->add(ExchgPid, commodityInfo, false);
            m_mapSessionCode[sid].insert(ExchgPid);
        }
    }
    Logger::info("commodities configuration file {} loaded", filename);
    root->release();
    return true;
}
bool CommonMgr::load_contracts(const char* filename)
{
    if (!FileUtils::exists(filename))
    {
        Logger::error("Contracts configuration file {} not exists", filename);
        return false;
    }
    Variant* root = CfgLoader::load_from_file(filename);
    if (!root)
    {
        Logger::error("Failed to load contracts configuration file {}", filename);
        return false;
    }
    for (const std::string& exchg : root->get_keys())
    {
        Variant* codeSubv = root->get(exchg);
        for (const std::string& code : codeSubv->get_keys())
        {
            Variant* field = codeSubv->get(code);
            const char* name = field->get_cstring("name");
            const char* exchg = field->get_cstring("exchg");
            CommodityInfo* cInfo = nullptr;
            const char* pid;
            if (field->has("product"))
            {
                pid = field->get_cstring("product");
                cInfo = get_commodity(field->get_cstring("exchg"), pid);
            }
            if (!cInfo)
            {
                Logger::warn("Commodity {}.{} not found, contract {} skipped", field->get_cstring("exchg"), pid, code.c_str());
                continue;
            }
            ContractInfo* contractInfo = ContractInfo::create(code.c_str(), name, exchg, pid);
            set_contract(contractInfo, field);
            ContractList* contractList = (ContractList*)m_mapExchgContract->get(exchg, false);
            if (!contractList)
            {
                contractList = ContractList::create();
                m_mapExchgContract->add(exchg, contractList, false);
            }
            contractList->add(code.c_str(), contractInfo, false);
            Array* contractArray = (Array*)m_mapContract->get(code, false);
            if(!contractArray)
            {
                contractArray = Array::create();
                m_mapContract->add(code, contractArray, false);
            }
            contractArray->append(contractInfo, true);    //数组则retain为true?
        }
    }
    Logger::info("Contracts configuration file {} loaded", filename);
    root->release();
    return true;

}
bool CommonMgr::load_holidays(const char* filename)
{
    if (!FileUtils::exists(filename))
    {
        Logger::error("Holiday configuration file {} not exists", filename);
        return false;
    }
    Variant* root = CfgLoader::load_from_file(filename);
    if (!root)
    {
        Logger::error("Failed to load holiday configuration file {}", filename);
        return false;
    }
    for (std::string& hid : root->get_keys())
    {
        Variant* holidays = root->get(hid);
        TradingDayTpl& trdDayTpl = m_mapTradingDayTpl[hid];
        for (uint32_t i = 0; i < holidays->size(); i++)
        {
            Variant* holiday = holidays->get(i);
            trdDayTpl.holidays.insert(holiday->as_uint32());
        }
    }
    root->release();
    Logger::info("Holiday configuration file {} loaded", filename);
    return true;
}

CommodityInfo* CommonMgr::get_commodity(const char* ExchgPid)
{
    return static_cast<CommodityInfo*>(m_mapCommodity->get(ExchgPid, false));
}
CommodityInfo* CommonMgr::get_commodity(const char* exchg, const char* pid)
{   //此处可用sstream
    std::string ExchgPid;
    ExchgPid = StrUtils::format("{}.{}", exchg, pid);
    return static_cast<CommodityInfo*>(m_mapCommodity->get(ExchgPid, false));
}
ContractInfo* CommonMgr::get_contract(const char* code, const char* exchg)
{
    //同时提供交易所id和合约id，通过ExchgContractMap查找---更快吗?
    if (strlen(exchg) != 0 && strlen(code) != 0)
    {
        auto it = m_mapExchgContract->find(exchg);
        if (it != m_mapExchgContract->end())
        {
            ContractList* contractList = static_cast<ContractList*>(it->second);
            auto it = contractList->find(code);
            if (it != contractList->end())
            {
                return static_cast<ContractInfo*>(it->second);
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }
    //只提供合约id，通过ContractMap查找    
    else if (strlen(code) != 0)
    {
        auto it = m_mapContract->find(code);
        if (it == m_mapContract->end())
        {
            return nullptr;
        }
        Array* contractArray = static_cast<Array*>(it->second);
        if (contractArray == nullptr || contractArray->size() == 0)
        {
            return nullptr;
        }
        return static_cast<ContractInfo*>(contractArray->at(0)); 
    }
    else
    {
        return nullptr;
    }
}
Array*          CommonMgr::get_contracts(const char* exchg /* = "" */)
{
    Array* retA = Array::create();
    if (strlen(exchg) != 0)
    {
        auto it = m_mapExchgContract->find(exchg);
        if (it != m_mapExchgContract->end())
        {
            ContractList* contractList = static_cast<ContractList*>(it->second);
            for (auto it = contractList->begin(); it != contractList->end(); it++)
            {
                retA->append(it->second, true);     //数组则retain为true?
            }
        }
    }
    else
    {
        for (auto it = m_mapExchgContract->begin(); it != m_mapExchgContract->end(); it++)
        {
            ContractList* contractList = static_cast<ContractList*>(it->second);
            for (auto it = contractList->begin(); it != contractList->end(); it++)
            {
                retA->append(it->second, true);     //数组则retain为true?
            }
        }
    }
    return retA;
}
SessionInfo* CommonMgr::get_session(const char* sid)
{
    return (SessionInfo*)m_mapSession->get(sid, false);
}
SessionInfo* CommonMgr::get_session_by_code(const char* code, const char* exchg/* = "" */)
{
    ContractInfo* contractInfo = get_contract(code, exchg);
    return contractInfo->get_comm_info()->get_session_info();
}
Array* CommonMgr::get_all_sessions()
{
    Array* retA = Array::create();
    for (auto it = m_mapSession->begin(); it != m_mapSession->end(); it++)
    {
        retA->append(it->second, true);
    }
    return retA;
}

uint32_t CommonMgr::calc_trading_date(const char* ExchgPid, uint32_t uDate, uint32_t uTime, bool isSession/* = false */)
{
    return 0;
}
bool CommonMgr::is_holiday(const char* ExchgPid, uint32_t uDate, bool isTpl /* = false */)
{
    uint32_t wd = TimeUtils::get_weekday_int(uDate);
    if (wd == 0 || wd == 6)
    {
        return true;
    }
    const char* tplid = isTpl ? ExchgPid : get_tplid(ExchgPid);
    auto it = m_mapTradingDayTpl.find(tplid);
    if (it != m_mapTradingDayTpl.end())
    {
        const TradingDayTpl& tpl = it->second;
        return (tpl.holidays.find(uDate) != tpl.holidays.end());
    }
    return false;

}
uint32_t CommonMgr::get_trading_date(const char* ExchgPid, uint32_t uOffDate /* = 0 */, bool isTpl /* = false */)
{
    const char* tplid = isTpl ? ExchgPid : get_tplid(ExchgPid);
    
    //如果没有记录交易日模板(节假日)，则返回当前日期
    uint32_t curDate = TimeUtils::get_cur_date_int();
    auto it = m_mapTradingDayTpl.find(tplid);
    if (it == m_mapTradingDayTpl.end())
    {
        return curDate;
    }

    TradingDayTpl* tpl = &(it->second);
    //如果没有偏移且有记录当前交易日，则返回
    if (tpl->cur_tdate != 0 && uOffDate == 0)
    {
        return tpl->cur_tdate;
    }

    if (uOffDate == 0)
    {
        uOffDate = curDate;
    }
    return uOffDate;
}
uint32_t	CommonMgr::get_next_tdate(const char* ExchgPid, uint32_t uDate, int days/* = 1 */, bool isTpl/* = false */)
{
    return 0;
}
uint32_t	CommonMgr::get_prev_tdate(const char* ExchgPid, uint32_t uDate, int days/* = 1 */, bool isTpl/* = false */)
{
    return 0;
}
bool		CommonMgr::is_trading_date(const char* ExchgPid, uint32_t uDate, bool isTpl/* = false */)
{
    return 0;
}
void		CommonMgr::set_trading_date(const char* ExchgPid, uint32_t uDate, bool isTpl/* = false */)
{
    return;
}


const char* CommonMgr::get_tplid(const char* ExchgPid)
{
    CommodityInfo* commInfo = get_commodity(ExchgPid);
    return commInfo->get_trdtpl();
}
void CommonMgr::set_session(SessionInfo* sessionInfo, Variant* field)
{
    if (field->has("auction"))
    {
        Variant* aution = field->get("auction");
        sessionInfo->set_auction_time(aution->get_uint32("from"), aution->get_uint32("to"));
    }
    Variant* sections = field->get("sections");
    if (!sections->is_array())
    {
        Logger::error("Invalid session section configuration");
        return;
    }
    for (uint32_t i = 0; i < sections->size(); ++i)
    {
        Variant* section = sections->get(i);
        sessionInfo->add_trading_section(section->get_uint32("from"), section->get_uint32("to"));
    }

}
void CommonMgr::set_commodity(CommodityInfo* commodityInfo, Variant* field)
{
    commodityInfo->set_vol_scale(field->get_uint32("volscale"));
    commodityInfo->set_price_tick(field->get_double("pricetick"));
    commodityInfo->set_category((ContractCategory)field->get_uint32("category"));
    commodityInfo->set_price_mode((PriceMode)field->get_uint32("pricemode"));
    if (field->has("tradingmode"))
    {
        commodityInfo->set_trading_mode((TradingMode)field->get_uint32("tradingmode"));
    }
    else
    {
        commodityInfo->set_trading_mode(TM_Both);
    }
	double lotsTick = 1;
	double minLots = 1;
	if (field->has("lotstick"))
		lotsTick = field->get_double("lotstick");
	if (field->has("minlots"))
		minLots = field->get_double("minlots");
	commodityInfo->set_lots_tick(lotsTick);
	commodityInfo->set_min_lots(minLots);
    SessionInfo* sessionInfo = get_session(field->get_cstring("session"));
    commodityInfo->set_session_info(sessionInfo);
}
void CommonMgr::set_contract(ContractInfo* contractInfo, Variant* field)
{
    
    const char* pid = field->get_cstring("product");
    CommodityInfo* commodityInfo = get_commodity(field->get_cstring("exchg"), pid);
    contractInfo->set_comm_info(commodityInfo);
    uint32_t maxMktQty = 1;
    uint32_t maxLmtQty = 1;
    uint32_t minMktQty = 1;
    uint32_t minLmtQty = 1;
    if (field->has("maxmarketqty"))
        maxMktQty = field->get_uint32("maxmarketqty");
    if (field->has("maxlimitqty"))
        maxLmtQty = field->get_uint32("maxlimitqty");
    if (field->has("minmarketqty"))
        minMktQty = field->get_uint32("minmarketqty");
    if (field->has("minlimitqty"))
        minLmtQty = field->get_uint32("minlimitqty");
    contractInfo->set_volume_limits(maxMktQty, maxLmtQty, minMktQty, minLmtQty);

}