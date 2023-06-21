#include "../CTPGateway.h"
bool CTPGateway::is_connected() 
{
    return m_gatewayState.load() == CS_Connected;

}