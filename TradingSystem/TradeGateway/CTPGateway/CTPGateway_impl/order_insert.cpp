#include "../CTPGateway.h"
#include "tools/Logger.h"
int CTPGateway::order_insert(Entrust* entrust) 
{
    if (entrust == nullptr)
    {
        Logger::error("CTPGateway::order_insert: entrust is nullptr");
        return -1;
    }
    if (m_gatewayState.load() != CS_Ready)
    {
        Logger::error("CTPGateway::order_insert: gateway is not ready");
    }
    Logger::info("order_insert: waiting gateway ready");
    std::unique_lock<std::mutex> lock(m_mtxConnect);
    m_cvConnect.wait(lock, [this]{return m_gatewayState.load() == CS_Ready;});
    req_order_insert(entrust);
    return 0;
}