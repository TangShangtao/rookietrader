#include "../CTPGateway.h"
#include "tools/Logger.h"
void CTPGateway::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    if (is_err_rspInfo(pRspInfo))
    {
        
        std::unique_lock<std::mutex> lock(m_mtxConnect);
        m_gatewayState.store(CS_Authenticated);
        Logger::info("user {} login failed with error msg {}", m_strUser, pRspInfo->ErrorMsg);//TODO: 转UTF-8输出
        m_cvConnect.notify_all();
        
    }
    else
    {
        //记录会话参数
        m_uFrontID = pRspUserLogin->FrontID;
        m_uSessionID = pRspUserLogin->SessionID;
        m_uTradingDay = strtoul(pRspUserLogin->TradingDay, nullptr, 10);
        m_uMaxOrderRef = strtoul(pRspUserLogin->MaxOrderRef, nullptr, 10);
        

        
        std::unique_lock<std::mutex> lock(m_mtxConnect);
        m_gatewayState.store(CS_Logged);
        Logger::info("user {} login successfully, tradingday {}, sessionID {}", m_strUser, m_uTradingDay, m_uSessionID);
        m_cvConnect.notify_all();
        
        
    }

}