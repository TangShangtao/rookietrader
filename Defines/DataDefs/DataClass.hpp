#pragma once
#include "DataStruct.h"
#include "../BaseDefs/BaseObject.hpp"
#include "../CommonDefs/ContractInfo.hpp"
#include <vector>
#include <algorithm>
NS_BEGIN

class BarData : public BaseObject
{
public:
    typedef std::vector<BarStruct> BarVec;

//protected
public:
    char        m_strCode[MAX_INSTRUMENT_LENGTH];
    BarPeriod   m_bpPeriod;
    uint32_t    m_uTimes;           //倍数?
    bool        m_bUnixTime;        //是否是时间戳格式,目前只在秒线上有效?
    BarVec      m_vecBarData;
    bool        m_bClosed;          //是否是闭合K线

public:
    BarData()
        :m_bpPeriod(BP_Min)
        ,m_uTimes(1)
        ,m_bUnixTime(false)
        ,m_bClosed(true)
    {

    }
    int32_t translate_idx(int32_t idx) const
    {
        if (idx < 0)
        {
            return std::max(0, (int32_t)m_vecBarData.size() + idx);
        }
        return idx;
    }
public:
    static BarData* create(const char* code, uint32_t size)
    {
        BarData* pret = new BarData;
        pret->m_vecBarData.resize(size);
        strcpy(pret->m_strCode, code);
        return pret;
    }
    virtual void release() override
    {
        if (get_ref_count() == 1)
        {
            m_vecBarData.clear();
        }
        BaseObject::release();
    }
public:
    void set_closed(bool bClosed){ m_bClosed = bClosed; }
    void set_unix_time(bool bEnabled){ m_bUnixTime = bEnabled; }
    void set_period(BarPeriod period, uint32_t times = 1) { m_bpPeriod = period; m_uTimes = times; }
    const char* get_code() const { return m_strCode; }
    bool is_closed() const { return m_bClosed; }
    bool is_unix_time() const { return m_bUnixTime; }
    BarPeriod get_period() const { return m_bpPeriod; }

};

class TickData : public PoolObject<TickData>
{
public:
    TickStruct  m_tickStruct;
    ContractInfo* m_pContract;
public:
    TickData() : m_pContract(nullptr) {}
    static TickData* create(const char* stdCode)
    {
        TickData* pret = TickData::allocate();
        auto len = strlen(stdCode);
        memcpy(pret->m_tickStruct.code, stdCode, len);
        pret->m_tickStruct.code[len] = 0;
        
        return pret;
    }
    static TickData* create(TickStruct& tickStruct)
    {
        TickData* pret = allocate();
        memcpy(&pret->m_tickStruct, &tickStruct, sizeof(TickStruct));
    }
    void set_contractInfo(ContractInfo* contractInfo) { m_pContract = contractInfo; }

};

NS_END;