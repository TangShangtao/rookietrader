#include "../CTPGateway.h"
#include "Utils/StrUtils.hpp"
#include "Utils/TimeUtils.hpp"
void CTPGateway::req_settlement_confirm() 
{
    CThostFtdcSettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    StrUtils::my_strncpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
    StrUtils::my_strncpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());
    StrUtils::format_to(req.ConfirmDate, "{}", m_uTradingDay);
    memcpy(req.ConfirmTime, TimeUtils::get_local_time_str().c_str(), 8);
    m_pCTPApi->ReqSettlementInfoConfirm(&req, generate_requestID());    
}