#include "../CTPGateway.h"
#include "Utils/StrUtils.hpp"
void CTPGateway::req_login() 
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	StrUtils::my_strncpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
	StrUtils::my_strncpy(req.UserID, m_strUser.c_str(), m_strUser.size());
	StrUtils::my_strncpy(req.Password, m_strPass.c_str(), m_strPass.size());
	StrUtils::my_strncpy(req.UserProductInfo, m_strProdInfo.c_str(), m_strProdInfo.size());
	m_pCTPApi->ReqUserLogin(&req, generate_requestID());
}