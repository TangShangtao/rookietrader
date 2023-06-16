#include "CTPGateway.h"
#include "../../../../Defines/Containers/Variant.hpp"
#include "../../../../TradingPlatformAPI/CTP6.3.15/ThostFtdcTraderApi.h"
#include "../../../../tools/Logger.h"


#include <sstream>
#include <boost/filesystem.hpp>


CTPGateway::CTPGateway()
{
}
CTPGateway::~CTPGateway()
{

}
//设置接口//
bool CTPGateway::init(Variant* cfg)
{
    auto front = cfg->get("front");
    if (front)
    {
        if (front->is_array())
        {
            for (uint32_t i = 0; i < front->size(); i++)
            {
                m_strFront.push_back(front->get(i)->as_cstring());
            }
        }
        else
        m_strFront.push_back(front->as_cstring());
    }
    else return false;
    m_strBroker = cfg->get_cstring("broker");
    m_strUser = cfg->get_cstring("user");
    m_strPass = cfg->get_cstring("pass");
    m_strAppID = cfg->get_cstring("appid");
    m_strAuthCode = cfg->get_cstring("authcode");
    m_strFlowDir = cfg->get_cstring("flowdir");
    m_strProdInfo = cfg->get_cstring("prodinfo");
    m_gatewayState = CS_DisConnected;
    return true;
}
bool CTPGateway::release()
{
    if (m_pCTPApi)
    {
        m_pCTPApi->Release();
        m_pCTPApi = nullptr;
    }
    if (m_ayOrders) m_ayOrders->clear();
    if (m_ayPosDetail) m_ayPosDetail->clear();
    if (m_mapPosition) m_mapPosition->clear();
    if ((m_ayTrades)) m_ayTrades->clear();    
}
void CTPGateway::register_spi(IGatewaySpi* listener)
{
    m_sink = listener;
    if (m_sink)
    {
        m_cmgr = listener->get_common_mgr();
    }
}

//连接接口//
void CTPGateway::connect()
{
    if (m_gatewayState != CS_DisConnected)
    {
        Logger::info("gateway is not initialized");
        return;
    }
    //创建流文件目录
    std::stringstream ss;
    ss << m_strFlowDir << "flows/" << m_strBroker << "/" << m_strUser << "/";
    boost::filesystem::create_directories(ss.str().c_str());
    //创建Api
    m_pCTPApi = CThostFtdcTraderApi::CreateFtdcTraderApi(ss.str().c_str());
    req_connect();
}
// //TODO
// void CTPGateway::disconnect()
// {
//     //将release函数加入工作线程, 等待工作线程执行完毕
//     release();
// }
// bool CTPGateway::is_connected()
// {

// }
int CTPGateway::login()
{
    //初始化得到帐号密码等配置信息后再登录
    if (m_gatewayState != CS_Connected)
    {
        Logger::info("gateway is not connected");
        return -1;
    }
    //要想登录, 首先需要认证, 等待认证结果回调
    req_authenticate();
    return 0;
}
int CTPGateway::logout()
{

}
//交易命令接口//




//实现CThostFtdcTraderSpi, 即CTP交易柜台通知时调用的回调接口; 含义请参考ThostFtdcTraderApi.h和CTP官方文档

void CTPGateway::OnFrontConnected()
{
    m_gatewayState = CS_Connected;
    if (m_sink)
    {
        m_sink->on_connect(true);
    }
    Logger::info("connected with front");

}
void CTPGateway::OnFrontDisconnected(int nReason)
{
    m_gatewayState = CS_DisConnected;
    if (m_sink)
    {
        m_sink->on_connect(false);
    }
    Logger::info("Disconnected with front with reason {}", nReason);
}
void CTPGateway::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!is_err_rspInfo(pRspInfo))
    {
        Logger::info("CTP Authentication succeed, now logging");
        req_login();
    }
    else
    {
        Logger::info("CTP Authentication failed: {}", pRspInfo->ErrorMsg);
    }
}
void CTPGateway::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!is_err_rspInfo(pRspInfo))
    {
        m_gatewayState = CS_Logged;
        Logger::info("login CTP succeed; Trading day: {}, User:{}, Broker:{}, now query settlement info", pRspUserLogin->TradingDay, pRspUserLogin->UserID, pRspUserLogin->BrokerID);
    //TODO初始化订单标记缓存器、委托单缓存器
    req_query_settlement_confirm();
    }
    else
    {
        m_gatewayState = CS_Connected;
        Logger::info("login CTP failed: {}", pRspInfo->ErrorMsg);
    }

}
//1.第一次确认 2.昨天已确认 3.今天已确认
void CTPGateway::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!is_err_rspInfo(pRspInfo))
    {
        m_gatewayState = CS_Confirming;
        Logger::info("query settlement info succeed");
    }

}


//gateway内部逻辑//
void CTPGateway::req_connect()
{

    
    //CTPApi的初始化工作 //TODO其他两种流重传方式
    if (!m_pCTPApi) return;
    m_pCTPApi->SubscribePublicTopic(THOST_TERT_QUICK);			//注册公有流
    m_pCTPApi->SubscribePrivateTopic(THOST_TERT_QUICK);		    //注册私有流
    for (std::string front : m_strFront)
    {
        m_pCTPApi->RegisterFront((char*)front.c_str());         //注册交易前置ip
        Logger::info("registerFront: {}", front);               
    }
    m_gatewayState = CS_Connecting;
    m_pCTPApi->Init();                                          //CTPApi初始化                                      
    // if (m_gatewayState == CS_Connected)
    // {

    // }
    //创建查询工作线程 //TODO线程创建
    // if (m_thrdWorker == nullptr)
    // {

    // }
}
void CTPGateway::req_authenticate()
{
    m_gatewayState = CS_Logging;
	CThostFtdcReqAuthenticateField req;
    memset(&req, 0, sizeof(req));
    strncpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
    strncpy(req.UserID, m_strUser.c_str(), m_strUser.size());
    strncpy(req.UserProductInfo, m_strProdInfo.c_str(), m_strProdInfo.size());
    strncpy(req.AuthCode, m_strAuthCode.c_str(), m_strAuthCode.size());
    strncpy(req.AppID, m_strAppID.c_str(), m_strAppID.size());
    m_pCTPApi->ReqAuthenticate(&req, generate_requestID());

}

void CTPGateway::req_login()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strncpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
	strncpy(req.UserID, m_strUser.c_str(), m_strUser.size());
	strncpy(req.Password, m_strPass.c_str(), m_strPass.size());
	strncpy(req.UserProductInfo, m_strProdInfo.c_str(), m_strProdInfo.size());
	m_pCTPApi->ReqUserLogin(&req, generate_requestID());
}

void CTPGateway::req_query_settlement_confirm()
{
    CThostFtdcQrySettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    strncpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
    strncpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());
    m_pCTPApi->ReqQrySettlementInfoConfirm(&req, generate_requestID());
}
void CTPGateway::req_settlement_confirm()
{

}


uint32_t CTPGateway::generate_requestID()
{
    return m_uRequestID.fetch_add(1) + 1;
}

bool is_err_rspInfo(CThostFtdcRspInfoField* pRspInfo)
{
    if (pRspInfo && pRspInfo->ErrorMsg != 0) return true;
    return false;
}