#include "../CTPGateway.h"
#include "tools/Logger.h"

int CTPGateway::confirm() 
{
    Logger::info("confirm: waiting onfrontconnected");
    std::unique_lock<std::mutex> lock(m_mtxConnect);
    m_cvConnect.wait(lock);
    if (m_gatewayState.load() == CS_Logged)
    {

        req_query_settlement_confirm();
    }
    else
    {
        Logger::info("confirm: CTP gateway is not logged, stop confirm");
        return -1;
    }
    Logger::info("confirm: waiting onrspquerysettlementconfirm");
    m_cvConnect.wait(lock);
    if (m_gatewayState.load() == CS_Queried)
    {
        req_settlement_confirm();
    }
    else if (m_gatewayState.load() == CS_Confirmed)
    {
        Logger::info("confirm: CTP gateway has confirmed settlement info");
        m_gatewayState.store(CS_Ready);
        Logger::info("confirm: CTP gateway is ready to trade");
        return 0;
    }
    else
    {
        Logger::info("confirm: CTP gateway query settlement info error, stop confirm");
        return -1;
    }
    return 0;
}