#include "../CTPGateway.h"
#include "tools/Logger.h"
#include <sstream>
#include <boost/filesystem.hpp>
int CTPGateway::connect() 
{
    //判断该接口是否可以被调用//
    if (CTPGateway::m_gatewayState.load() != CS_DisConnected)
    {
        Logger::error("connect: CTP gateway is not disconnected");
        return -1;
    }
    //gateway层面的工作//
    //创建流文件目录
    std::stringstream ss;
    ss << m_strFlowDir << "flows/" << m_strBroker << "/" << m_strUser << "/";
    boost::filesystem::create_directories(ss.str().c_str());
    //创建Api
    m_pCTPApi = CThostFtdcTraderApi::CreateFtdcTraderApi(ss.str().c_str());
    m_pCTPApi->RegisterSpi(this);
    //调用CTPApi的接口下达请求//
    req_connect();

    return 0;
}
