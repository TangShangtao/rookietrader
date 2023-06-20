#include "CTPGateway.h"
#include "Defines/Containers/Variant.hpp"
#include "TradingPlatformAPI/CTP6.3.15/ThostFtdcTraderApi.h"
#include "tools/Logger.h"
#include "Utils/TimeUtils.hpp"
#include "Utils/StrUtils.hpp"

#include <sstream>
#include <boost/filesystem.hpp>
#include <iostream>
CTPGateway::CTPGateway()
{
}
CTPGateway::~CTPGateway()
{
}
////实现IGatewayApi, 即gateway对外提供的交易接口, 由adapter调用////

//设置接口//
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

int CTPGateway::release() 
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
    return 0;
}
// void CTPGateway::register_spi(IGatewaySpi* listener)
// {
//     return;
//     // m_sink = listener;
//     // if (m_sink)
//     // {
//     //     m_cmgr = listener->get_common_mgr();
//     // }
// }
void CTPGateway::join() 
{
    m_pCTPApi->Join();
}
//执行交易命令前的准备工作: 与柜台连接、登录、确认结算单信息//
int CTPGateway::connect() 
{
    //判断该接口是否可以被调用//
    if (CTPGateway::m_gatewayState.load() != CS_DisConnected)
    {
        Logger::error("CTP gateway is not disconnected");
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

int CTPGateway::disconnect() 
{
    return 0;
}

bool CTPGateway::is_connected() 
{
    return false;
}

int CTPGateway::login() 
{
    
    return 0;
}

int CTPGateway::logout() 
{
    return 0;
}
//交易命令接口//
int CTPGateway::order_insert(Entrust* entrust) 
{
    return 0;
}

int CTPGateway::order_action(EntrustAction* action) 
{
    return 0;
}

bool CTPGateway::make_entrustID(char* buffer, int length) 
{
    return false;
}
//查询信息接口//
int CTPGateway::query_account() 
{
    return 0;
}

int CTPGateway::query_positions() 
{
    return 0;
}

int CTPGateway::query_orders() 
{
    return 0;
}

int CTPGateway::query_trades() 
{
    return 0;
}

int CTPGateway::query_settlement(uint32_t uDate) 
{
    return 0;
}

////gateway与CTP交互的内部逻辑, gateway直接调用CTP提供的Api的部分写在这里////

//执行交易命令前的准备工作: 与柜台连接、登录、确认结算单信息//
void CTPGateway::req_connect() 
{
    //CTPApi的初始化工作 //TODO其他两种流重传方式
    if (!m_pCTPApi) return;
    m_pCTPApi->SubscribePublicTopic(THOST_TERT_QUICK);			//注册公有流
    m_pCTPApi->SubscribePrivateTopic(THOST_TERT_QUICK);		    //注册私有流
    m_pCTPApi->RegisterFront((char*)m_strFront.c_str());         //注册交易前置ip
    Logger::info("registerFront: {}", m_strFront.c_str());    
    m_pCTPApi->Init();                                          //CTPApi初始化    

}
void CTPGateway::req_authenticate() 
{
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
    CThostFtdcSettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    strncpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
    strncpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());
    StrUtils::format_to(req.ConfirmDate, "{}", TimeUtils::get_cur_date_int());
    memcpy(req.ConfirmTime, TimeUtils::get_local_time_str().c_str(), 8);
    m_pCTPApi->ReqSettlementInfoConfirm(&req, generate_requestID());    
}

////CTP以回调函数的参数的形式传递报文, 通过实现CTP回调函数来接收报文和执行下一步操作////

//执行交易命令前的准备工作: 与柜台连接、登录、确认结算单信息//
void CTPGateway::OnFrontConnected() 
{
    CTPGateway::m_gatewayState.store(CS_Connected);
    // // if (m_sink)
    // // {
    // //     m_sink->on_connect(true);
    // // }
    // Logger::info("connected with front");

    std::cout << std::this_thread::get_id() << std::endl;
    
}

void CTPGateway::OnFrontDisconnected(int nReason) 
{
    m_gatewayState.store(CS_DisConnected);
    m_gatewayState = CS_DisConnected;
    // if (m_sink)
    // {
    //     m_sink->on_connect(false);
    // }
    Logger::info("Disconnected with front with reason {}", nReason);
}

void CTPGateway::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPGateway::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPGateway::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPGateway::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPGateway::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

//CTP接受交易命令后的回报
void CTPGateway::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPGateway::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPGateway::OnRtnOrder(CThostFtdcOrderField *pOrder) {}

void CTPGateway::OnRtnTrade(CThostFtdcTradeField *pTrade) {}

void CTPGateway::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) {}

//CTP接受查询命令后的回报
void CTPGateway::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPGateway::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPGateway::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPGateway::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPGateway::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

//CTP的其他回调函数
void CTPGateway::OnHeartBeatWarning(int nTimeLapse) {}

void CTPGateway::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPGateway::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus) {}

////gateway实现的过程中常用的逻辑封装写在这里////

//产生请求编号//
uint32_t CTPGateway::generate_requestID()
{
    return -1;
}
//检验CTP的回应是否为错误//
bool CTPGateway::is_err_rspInfo(CThostFtdcRspInfoField *pRspInfo)
{
    return false;
}