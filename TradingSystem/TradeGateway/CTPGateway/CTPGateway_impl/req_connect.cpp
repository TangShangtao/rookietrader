#include "../CTPGateway.h"
#include "tools/Logger.h"
void CTPGateway::req_connect() 
{
    //CTPApi的初始化工作 //TODO其他两种流重传方式
    if (!m_pCTPApi) return;
    m_pCTPApi->SubscribePublicTopic(THOST_TERT_QUICK);			//注册公有流
    m_pCTPApi->SubscribePrivateTopic(THOST_TERT_QUICK);		    //注册私有流
    m_pCTPApi->RegisterFront((char*)m_strFront.c_str());         //注册交易前置ip
    Logger::info("req_connect: registerFront: {}", m_strFront.c_str());    
    m_pCTPApi->Init();                                          //CTPApi初始化    

}