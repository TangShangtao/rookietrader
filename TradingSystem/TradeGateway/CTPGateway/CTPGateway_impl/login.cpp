#include "../CTPGateway.h"
#include "tools/Logger.h"
int CTPGateway::login() 
{
    
    std::unique_lock<std::mutex> lock(m_mtxConnect);
    m_cvConnect.wait(lock);
    if (m_gatewayState.load() == CS_Connected)
    {
        req_authenticate();
    }
    else
    {
        Logger::info("CTP gateway is not connected, stop login");
        return -1;
    }
    
    m_cvConnect.wait(lock);
    if (m_gatewayState.load() == CS_Authenticated)
    {
        req_login();
    }
    else
    {
        Logger::info("CTP gateway is not authenticated, stop login");
        return -1;
    }
    return 0;
}