#include "HisDataReplayer.h"
#include "../tools/Logger.h"


bool HisDataReplayer::load_common(Variant* cfg)
{
    Variant* commonCfg = cfg->get("basefiles");
    const char* sessionPath = commonCfg->get_cstring("session");
    bool ret_session = m_commonMgr.load_sessions(sessionPath);
    const char* commodityPath = commonCfg->get_cstring("commodity");
    bool ret_commodity = m_commonMgr.load_commodities(commodityPath);
    const char* contractPath = commonCfg->get_cstring("contract");
    bool ret_contract = m_commonMgr.load_contracts(contractPath);
    const char* holidayPath = commonCfg->get_cstring("holiday");
    bool ret_holiday = m_commonMgr.load_holidays(holidayPath);

    //hot & second & rules & fees & adjfactor
    if (ret_session && ret_commodity && ret_contract && ret_holiday)
    {
        Logger::info("common file loaded");
        return true;
    }
    else
    {
        Logger::info("failed to load commonfile");
        return false;
    }
    
    


}
bool HisDataReplayer::init(Variant* cfg)
{
    if (!load_common(cfg))
    {
        return false;
    }
    m_uBeginTime = cfg->get_int64("stime");
    m_uEndTime = cfg->get_int64("etime");
    Logger::info("backtest time range: {} to {}", m_uBeginTime, m_uEndTime);
    m_bTickEnabled = cfg->get_bool("tick");
    Logger::info("tick data replay is {}", m_bTickEnabled? "enabled" : "disabled");
    m_strHisDataFormat = cfg->get_cstring("format");
    m_strHisDataPath = cfg->get_cstring("path");
    return true;
}

void HisDataReplayer::sub_tick(uint32_t ctxId, const char* stdCode)
{
    //策略只能订阅tick数据，若开启tick回测，则回放tick数据
    if (m_bTickEnabled)
    {
        m_tick_sub_map[stdCode] = ctxId;
    }
    //否则用bar数据模拟tick数据再回放
    else
    {
        m_bar_sub_map[stdCode] = ctxId;
    }
    
}

bool HisDataReplayer::check_ticks(uint32_t uDate)
{
    bool bHasTick = false;
    for (auto& val : m_tick_sub_map)
    {
        const char* stdCode = val.first.c_str();
        if (strlen(stdCode) == 0) return false;//没有订阅
        bool bNeedCache = false;
        auto it = m_tick_cache.find(stdCode);
        if (it == m_tick_cache.end()) bNeedCache = true;//缓存为空
        else
        {
            HftDataList<TickStruct>& tickList = it->second;//(HftDataList<TickStruct>&)
            if (tickList._date != uDate)                //缓存为上一日的数据
            {
                tickList._items.clear();
                bNeedCache = true;
            }
            else if (tickList._count == 0)              //缓存失败
            {
                return false;
            }
            //已缓存
        }   
        if (bNeedCache)
        {
            bool bSucc = false;
            if (m_strHisDataFormat == "csv")
            {
                bSucc = cache_ticks_from_csv(stdCode, uDate);
            }
            else
            {
                bSucc = cache_ticks_from_dsb(stdCode, uDate);
            }
            if (!bSucc)
            {
                auto& tickList = m_tick_cache[stdCode];
                tickList._items.resize(0);
                tickList._cursor = UINT_MAX;
                tickList._date = uDate;
                tickList._count = 0;
                return false;
            }
        }
    }
    return true;
}
//1.有dsb数据则直接读取
//2.没有dsb数据则读取csv数据
//3.若读取csv数据，转储一份dsb数据
bool HisDataReplayer::cache_bars_from_csv(const char* stdCode, BarPeriod period, bool bSubbed)
{
    
}