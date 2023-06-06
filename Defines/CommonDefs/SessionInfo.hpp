//记录每天交易时间段信息, 来源于配置文件
#pragma once
#include "../ConstantDefs/Marcos.h"
#include "../BaseDefs/BaseObject.hpp"

#include <utility>
#include <vector>
#include <string>

NS_BEGIN
class SessionInfo : public BaseObject
{;
public:
    typedef std::pair<uint32_t, uint32_t> TradingSection;
    typedef std::vector<TradingSection> TradingTimes;
public:
    int32_t m_uOffsetMins;          //夜盘offset为300min，白盘没有offset
    std::string m_strSid;           //FD0900
    std::string m_strName;          //期货白盘 
    TradingTimes m_tradingTimes;    //交易时间段
    TradingSection m_auctionTime;   //集合竞价时间段
    

public:
    // 此处与wondertrader不同
    static SessionInfo* create(const char* Sid, const char* name, int32_t offsetMins = 0)
    {
        SessionInfo* pRet = new SessionInfo;
        pRet->m_strSid = Sid;
        pRet->m_strName = name;
        pRet->m_uOffsetMins = offsetMins;
        return pRet;
    }
    void add_trading_section(uint32_t sTime, uint32_t eTime)
    {
        sTime = offset_time(sTime, true);
        eTime = offset_time(eTime, false);
        m_tradingTimes.emplace_back(TradingSection(sTime, eTime));
    }
    void set_auction_time(uint32_t sTime, uint32_t eTime)
    {
        sTime = offset_time(sTime, true);
        eTime = offset_time(eTime, false);
        m_auctionTime.first = sTime;
        m_auctionTime.second = eTime;
    }

    const char* get_id() const { return m_strSid.c_str(); }
    const char* get_name() const { return m_strName.c_str(); }
    int32_t get_offset_mins() const { return m_uOffsetMins; }



public:
    uint32_t offset_time(uint32_t uTime, bool bAlignLeft) const
    {
        uint32_t curMinute = (uTime / 100) * 60 + uTime % 100;
        curMinute += m_uOffsetMins;
        //左对齐: 
		curMinute += m_uOffsetMins;
		if(bAlignLeft)
		{
			if (curMinute >= 1440)
				curMinute -= 1440;
			else if (curMinute < 0)
				curMinute += 1440;
		}
		else
		{
			if (curMinute > 1440)
				curMinute -= 1440;
			else if (curMinute <= 0)
				curMinute += 1440;
		}
		
		return (curMinute/60)*100 + curMinute%60;
    }

};



NS_END;
