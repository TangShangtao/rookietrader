#include "../CTPGateway.h"
#include "Utils/StrUtils.hpp"
void CTPGateway::req_authenticate() 
{
	CThostFtdcReqAuthenticateField req;
    memset(&req, 0, sizeof(req));
    StrUtils::my_strncpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
    StrUtils::my_strncpy(req.UserID, m_strUser.c_str(), m_strUser.size());
    StrUtils::my_strncpy(req.UserProductInfo, m_strProdInfo.c_str(), m_strProdInfo.size());
    StrUtils::my_strncpy(req.AuthCode, m_strAuthCode.c_str(), m_strAuthCode.size());
    StrUtils::my_strncpy(req.AppID, m_strAppID.c_str(), m_strAppID.size());
    m_pCTPApi->ReqAuthenticate(&req, generate_requestID());    
}