#include "../CTPGateway.h"
#include "tools/Logger.h"

void CTPGateway::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    if (is_err_rspInfo(pRspInfo)) 
    {
        Logger::error("Settlement info confirm failed with error msg : {}", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        std::unique_lock<std::mutex> lock(m_mtxConnect);
        m_gatewayState.store(CS_Queried);
        m_cvConnect.notify_all();
        
    }
    else
    {
        Logger::info("Settlement info confirm successfully");
        std::unique_lock<std::mutex> lock(m_mtxConnect);
        m_gatewayState.store(CS_Confirmed);
        m_gatewayState.store(CS_Ready);
        Logger::info("CTP gateway is ready to trade");
        m_cvConnect.notify_all();
    }
}

