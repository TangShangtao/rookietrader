#include "../CTPGateway.h"
#include "tools/Logger.h"
int CTPGateway::order_insert(Entrust* entrust) 
{
    if (entrust == nullptr)
    {
        Logger::error("CTPGateway::order_insert: entrust is nullptr");
    }
    std::unique_lock<std::mutex> lock(m_mtxConnect);
    m_cvConnect.wait(lock, [this] {return m_gatewayState.load() == CS_Ready; });
    lock.unlock();
    req_order_insert(entrust);
}