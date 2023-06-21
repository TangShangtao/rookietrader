#include "../CTPGateway.h"
#include "tools/Logger.h"

void CTPGateway::OnFrontDisconnected(int nReason) 
{
    Logger::info("Disconnected with front {} with reason {}", m_strFront, nReason);
    std::unique_lock<std::mutex> lock(m_mtxConnect);
    m_gatewayState.store(CS_DisConnected);
    m_cvConnect.notify_all();

}