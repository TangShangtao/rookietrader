#include "../CTPGateway.h"
#include "tools/Logger.h"
void CTPGateway::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    if (is_err_rspInfo(pRspInfo))
    {
        Logger::info("query settlement info confirm failed with error msg {}", pRspInfo->ErrorMsg);//TODO: 转UTF-8输出
        std::unique_lock<std::mutex> lock(m_mtxConnect);
        m_gatewayState.store(CS_Logged);
        m_cvConnect.notify_all();
    }
    else
    {
        if (pSettlementInfoConfirm == nullptr)
        {
            Logger::info("Never confirmed settlement info before, now confirming");
            std::unique_lock<std::mutex> lock(m_mtxConnect);
            m_gatewayState.store(CS_Queried);
            m_cvConnect.notify_all();
        }
        else
        {
            uint32_t uConfirmDate = strtoul(pSettlementInfoConfirm->ConfirmDate, nullptr, 10);
            if (uConfirmDate == m_uTradingDay)
            {
                Logger::info("Settlement info has been confirmed today");
                std::unique_lock<std::mutex> lock(m_mtxConnect);
                m_gatewayState.store(CS_Confirmed);
                m_cvConnect.notify_all();
            }
            else if (uConfirmDate < m_uTradingDay)
            {
                Logger::info("Settlement info has been confirmed on {}", uConfirmDate);
                std::unique_lock<std::mutex> lock(m_mtxConnect);
                m_gatewayState.store(CS_Queried);
                m_cvConnect.notify_all();
            }
            else
            {
                Logger::error("Settlement info has been confirmed on {}, but today is {}", uConfirmDate, m_uTradingDay);
                std::unique_lock<std::mutex> lock(m_mtxConnect);
                m_gatewayState.store(CS_Logged);
                m_cvConnect.notify_all();
            }
            
        }
    }

}