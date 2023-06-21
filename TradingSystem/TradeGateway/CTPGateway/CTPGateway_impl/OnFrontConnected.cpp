#include "../CTPGateway.h"
#include "tools/Logger.h"
void CTPGateway::OnFrontConnected() 
{
    Logger::info("Connected with front {}", m_strFront);
    std::unique_lock<std::mutex> lock(m_mtxConnect);
    m_gatewayState.store(CS_Connected);
    m_cvConnect.notify_all();
    
}