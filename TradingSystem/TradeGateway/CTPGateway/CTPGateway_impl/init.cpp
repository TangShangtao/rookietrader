#include "../CTPGateway.h"
#include "tools/Logger.h"
int CTPGateway::init(Variant* cfg) 
{
    m_strFront = cfg->get_cstring("front");
    m_strBroker = cfg->get_cstring("broker");
    m_strUser = cfg->get_cstring("user");
    m_strPass = cfg->get_cstring("pass");
    m_strAppID = cfg->get_cstring("appid");
    m_strAuthCode = cfg->get_cstring("authcode");
    m_strFlowDir = cfg->get_cstring("flowdir");
    m_strProdInfo = cfg->get_cstring("prodinfo");
    m_gatewayState.store(CS_DisConnected);
    Logger::info("CTP gateway initialized");
    return 0;
}