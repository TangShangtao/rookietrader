#include "../CTPGateway.h"
#include "Utils/StrUtils.hpp"
void CTPGateway::req_query_settlement_confirm() 
{
    CThostFtdcQrySettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    StrUtils::my_strncpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
    StrUtils::my_strncpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());
    m_pCTPApi->ReqQrySettlementInfoConfirm(&req, generate_requestID());
}