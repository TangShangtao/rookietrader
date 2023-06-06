//记录合约信息, 来源于配置文件
//如IF2310
#pragma once
#include "../ConstantDefs/Marcos.h"
#include "../BaseDefs/BaseObject.hpp"
#include "../ConstantDefs/Types.h"

#include <string>
#include <sstream>

NS_BEGIN

class CommodityInfo;

class ContractInfo : public BaseObject
{
public:
    std::string m_strCode;      //rb2312
    std::string m_strExchg;     //SHFE
    std::string m_strName;      //螺纹2312
    std::string m_strProduct;   //rb

    std::string m_strExchgPid;   //SHFE.rb
    std::string m_strExchgCode;  //SHFE.rb2312

    uint32_t m_maxMktQty;
    uint32_t m_maxLmtQty;
    uint32_t m_minMktQty;
    uint32_t m_minLmtQty;

    CommodityInfo* m_pCommInfo;

public:
    static ContractInfo* create(const char* code, const char* name, const char* exchg, const char* pid)
    {
		ContractInfo* ret = new ContractInfo;
		ret->m_strCode = code;
		ret->m_strName = name;
		ret->m_strProduct = pid;
		ret->m_strExchg = exchg;

		std::stringstream ss;
		ss << exchg << "." << code;
		ret->m_strExchgCode = ss.str();

		std::stringstream sss;
		sss << exchg << "." << pid;
		ret->m_strExchgPid = sss.str();

		return ret;
    }
    void set_volume_limits(uint32_t maxMktQty, uint32_t maxLmtQty, uint32_t minMktQty, uint32_t minLmtQty)
    {
        m_maxMktQty = maxMktQty;
        m_maxLmtQty = maxLmtQty;
        m_minMktQty = minMktQty;
        m_minLmtQty = minLmtQty;
    }
    const char* get_code() const { return m_strCode.c_str(); }
    const char* get_name() const { return m_strName.c_str(); }
    const char* get_exchg() const { return m_strExchg.c_str(); }
    const char* get_product() const { return m_strProduct.c_str(); }
    const char* get_full_code() const { return m_strExchgCode.c_str(); }
    const char* get_full_pid() const { return m_strExchgPid.c_str(); }
    uint32_t get_max_mkt_qty() const { return m_maxMktQty; }
    uint32_t get_max_lmt_qty() const { return m_maxLmtQty; }
    uint32_t get_min_mkt_qty() const { return m_minMktQty; }
    uint32_t get_min_lmt_qty() const { return m_minLmtQty; }
    void set_comm_info(CommodityInfo* pCommInfo) { m_pCommInfo = pCommInfo; }
    CommodityInfo* get_comm_info() const { return m_pCommInfo; }
};


NS_END;