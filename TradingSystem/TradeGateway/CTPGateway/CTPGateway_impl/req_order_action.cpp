#include "../CTPGateway.h"
#include "string.h"
#include "Utils/StrUtils.hpp"
#include "Defines/TradeDefs/EntrustAction.hpp"
#include "tools/Logger.h"
void CTPGateway::req_order_action(EntrustAction* action)
{
    uint32_t uFrontID = 0;
    uint32_t uSessionID = 0;
    uint32_t uOrderRef = 0;
    if (!extract_entrustID(action->get_entrustID(), uFrontID, uSessionID, uOrderRef))
    {
        Logger::error("CTPGateway::req_order_action: extract_entrustID failed");
        return;
    }
    CThostFtdcInputOrderActionField req;
    memset(&req, 0, sizeof(req));
    StrUtils::my_strncpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
    StrUtils::my_strncpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());
    StrUtils::format_to(req.OrderRef, "{}", uOrderRef);
    req.FrontID = uFrontID;
    req.SessionID = uSessionID;
    req.ActionFlag = action_flag_to_CTP(action->get_action_flag());
    StrUtils::my_strncpy(req.ExchangeID, action->get_exchg());
    StrUtils::my_strncpy(req.InstrumentID, action->get_code());
    req.LimitPrice = action->get_price();
    req.VolumeChange = (int32_t)action->get_volume();

    //TODO
    StrUtils::my_strncpy(req.OrderSysID, action->get_orderID());
    int ret = m_pCTPApi->ReqOrderAction(&req, generate_requestID());
    if (ret != 0)
    {
        Logger::error("CTPGateway::req_order_action: ReqOrderAction failed, error code: {}", ret);
    }


}