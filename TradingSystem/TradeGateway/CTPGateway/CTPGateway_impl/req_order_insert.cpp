#include "../CTPGateway.h"
#include "Defines/TradeDefs/Entrust.hpp"
#include "Utils/StrUtils.hpp"
#include "tools/Logger.h"
void CTPGateway::req_order_insert(Entrust* entrust)
{
    CThostFtdcInputOrderField req;
    memset(&req, 0, sizeof(req));
    //基础信息
    StrUtils::my_strncpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
    StrUtils::my_strncpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());
    StrUtils::my_strncpy(req.InstrumentID, entrust->get_code());
    StrUtils::my_strncpy(req.ExchangeID, entrust->get_exchg());
    StrUtils::my_strncpy(req.UserID, m_strUser.c_str(), m_strUser.size());
    //委托信息 TODO: 字段含义、哪些是必须的

    //1.直接使用CTPGateway下单, 本地订单ID由CTPGateway生成
    if (strlen(entrust->get_entrustID()) == 0)
    {
        generate_entrustID(entrust->get_entrustID(), sizeof(entrust->get_entrustID()));
        StrUtils::format_to(req.OrderRef, "{}", m_uOrderRef.load());
    }
    //TODO2.基于TradeAdapter下单, 本地订单ID由TradeAdapter生成
    else
    {
        StrUtils::format_to(req.OrderRef, "{}", m_uOrderRef.load());
    }
    
    
    req.OrderPriceType = price_type_to_CTP(entrust->get_price_type(), strcmp(entrust->get_exchg(), "CFFEX") == 0);
    req.Direction = direction_type_to_CTP(entrust->get_direction(), entrust->get_offset_type());
    req.CombOffsetFlag[0] = offset_type_to_CTP(entrust->get_offset_type());
    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    req.LimitPrice = entrust->get_price();
    req.VolumeTotalOriginal = entrust->get_volume();
	if(entrust->get_order_flag() == OF_NOR)
	{
		req.TimeCondition = THOST_FTDC_TC_GFD;
		req.VolumeCondition = THOST_FTDC_VC_AV;
	}
	else if (entrust->get_order_flag() == OF_FAK)
	{
		req.TimeCondition = THOST_FTDC_TC_IOC;
		req.VolumeCondition = THOST_FTDC_VC_AV;
	}
	else if (entrust->get_order_flag() == OF_FOK)
	{
		req.TimeCondition = THOST_FTDC_TC_IOC;
		req.VolumeCondition = THOST_FTDC_VC_CV;
	}
    req.MinVolume = 1;
    req.ContingentCondition = THOST_FTDC_CC_Immediately;
    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    req.IsAutoSuspend = 0;
    req.UserForceClose = 0;
    req.IsSwapOrder = 0;
    //请求报单
    int ret = m_pCTPApi->ReqOrderInsert(&req, generate_requestID());
    if (ret != 0)
    {
        Logger::error("CTPGateway::req_order_insert: ReqOrderInsert failed, errorID: %d", ret);
    }

}