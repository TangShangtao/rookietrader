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
        return -1;
    }
    req_order_insert(entrust);
    return 0;
}