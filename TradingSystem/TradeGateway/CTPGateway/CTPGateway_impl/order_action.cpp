#include "../CTPGateway.h"
#include "tools/Logger.h"
int CTPGateway::order_action(EntrustAction* action) 
{
    if (action == nullptr)
    {
        Logger::error("order_action: action is nullptr");
        return -1;
    }
    if (m_gatewayState.load() != CS_Ready)
    {
        Logger::error("order_action: gateway is not ready");
        return -1;
    }
    req_order_action(action);
    return 0;
    
}