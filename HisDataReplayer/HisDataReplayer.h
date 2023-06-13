//历史数据回放器
//1.读取数据，包括历史行情数据(csv/dsb & tick/bar)和基本配置信息
//2.回放数据产生事件，调用事件回调
//3.提供数据查询接口
#pragma once
#include "CommonMgr.h"
#include "IEventCallbacks.h"
#include "../Defines/Containers/DataList.hpp"
#include "../DataDefs/DataClass.hpp"
#include "../DataDefs/DataStruct.h"
#include <string>
#include <unordered_map>

NS_BEGIN
class TickSlice;
class BarSlice;
NS_END;
USING_NS;

class HisDataReplayer
{
public:

    //历史行情数据缓存
    typedef std::unordered_map<std::string, HftDataList<TickStruct>>    TickCache;//{stdCode : vec}
    typedef std::unordered_map<std::string, BarDataList>                BarCache;
    //策略订阅行情数据
    typedef std::unordered_map<std::string, uint32_t>                   StraSubMap;//{stdCode : Ctxid}

public:
    //回放器的主要组成类
    CommonMgr           m_commonMgr;
    IEventCallbacks*    m_listener;//抽象类必须用指针形式触发多态
    //历史行情数据订阅&缓存
    StraSubMap          m_tick_sub_map;
    StraSubMap          m_bar_sub_map;
    TickCache           m_tick_cache;
    BarCache            m_bar_cache;
    //回放器设置
    uint64_t            m_uBeginTime;
    uint64_t            m_uEndTime;
    std::string         m_strHisDataFormat;
    std::string         m_strHisDataPath;
    bool                m_bTickEnabled;
    //回放器运行标识
    bool                m_bRunning;

public:
    //1.读取基本配置信息
    bool            load_common(Variant* cfg);
    //1.读取历史数据到缓存
    void            sub_tick(uint32_t ctxId, const char* stdCode);//策略订阅所需数据
    // void            sub_bar(uint32_t ctxId, const char* stdCode); //策略订阅所需数据
    bool            check_ticks(uint32_t uDate);//检查一日数据缓存
    // bool            check_bars(uint32_t uDate); //检查一日数据缓存
    bool            cache_ticks_from_dsb(const char* stdCode, uint32_t uDate);//读取一日tick数据到缓存
    bool            cache_ticks_from_csv(const char* stdCode, uint32_t uDate);//读取一日tick数据到缓存
    bool            cache_bars_from_dsb(const char* stdCode, BarPeriod period);//读取全部bar数据到缓存
    bool            cache_bars_from_csv(const char* stdCode, BarPeriod period);//读取全部bar数据到缓存

    //2.回放历史数据，调用事件回调
    void            run_by_bars();
    void            run_by_ticks();
    //3.提供数据查询接口
    BarSlice*       get_bar_slice(const char* stdCode, BarPeriod period, uint32_t count, uint32_t times = 1, bool isMain = false);
    TickSlice*      get_tick_slice(const char* stdCode, uint32_t count, uint64_t etime = 0);
    SessionInfo*    get_session_info(const char* sid, bool isCode = false);
    CommodityInfo*  get_commodity_info(const char* stdCode);
public:
    //入口接口
    bool init(Variant* cfg);
    bool prepare();
    bool run();
    void stop();
public:
    //封装内部逻辑
    void register_callbacks(IEventCallbacks* listener) { m_listener = listener; }
public:
    HisDataReplayer();
    ~HisDataReplayer();

};