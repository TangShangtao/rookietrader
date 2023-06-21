#include "../CTPGateway.h"
#include "tools/Logger.h"
void CTPGateway::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    if (is_err_rspInfo(pRspInfo))
    {
        Logger::info("Authenticate failed with error msg {}", pRspInfo->ErrorMsg);//TODO: 转UTF-8输出
        std::unique_lock<std::mutex> lock(m_mtxConnect);
        m_gatewayState.store(CS_Connected);
        m_cvConnect.notify_all();
    }
    else
    {
        Logger::info("Authenticated, authcode : {}", m_strAuthCode);
        std::unique_lock<std::mutex> lock(m_mtxConnect);
        m_gatewayState.store(CS_Authenticated);
        m_cvConnect.notify_all();
    }
    
}