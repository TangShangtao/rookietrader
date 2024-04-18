#include "mdctp.h"
namespace rookietrader
{
MDCTP::MDCTP(const nlohmann::json& config)
    :   MDService(config),
        ctpMdApi(CThostFtdcMdApi::CreateFtdcMdApi())
{
    std::string frontAddrCopy = frontAddr;
    ctpMdApi->RegisterFront(frontAddrCopy.data());
    ctpMdApi->RegisterSpi(this);
    logger.debug("MDCTP::MDCTP,called");
}

MDCTP::~MDCTP()
{
    WaitReq();
    ctpMdApi->Join();
    ctpMdApi->Release();
    logger.debug("MDCTP::~MDCTP,called");
}

bool MDCTP::OnPrepareMDReq(const PrepareMDReq* req) 
{   
    ctpMdApi->Init();
    logger.info("MDCTP::OnPrepareMDReq,ctpMdApi->Init() called");
    return true;

}   
bool MDCTP::OnSubTickReq(const SubTickReq* subTickReq) 
{
    return true;
}

void MDCTP::OnFrontConnected() 
{
    CThostFtdcReqUserLoginField login;
    std::memset(&login, 0, sizeof(login));
    std::strncpy(login.UserID, accountID.c_str(), sizeof(login.UserID)-1);
    std::strncpy(login.Password, password.c_str(), sizeof(login.Password)-1);
    std::strncpy(login.BrokerID, "9999", sizeof(login.BrokerID)-1);
    ctpMdApi->ReqUserLogin(&login, prepareMDRpcID);
    logger.info("MDCTP::OnFrontConnected,call ReqUserLogin; reqID {}", prepareMDRpcID);
}

void MDCTP::OnFrontDisconnected(int nReason) 
{
    logger.error("MDCTP::OnFrontDisconnected,called; nReason {}", nReason);
}

void MDCTP::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    if (CheckError(pRspUserLogin, pRspInfo, "OnRspUserLogin") == false)
    {
        SendPrepareMDRsp(false, "OnRspUserLogin failed");
        logger.error("MDCTP::OnRspUserLogin, MDCTP failed to prepare");
        exit(-1);
        return;
    }
    SendPrepareMDRsp(true, "");
    PublishMDReady();
    logger.info("MDCTP::OnRspUserLogin, MDCTP prepared!");
}

void MDCTP::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{

}

void MDCTP::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{

}

void MDCTP::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) 
{

}

bool MDCTP::CheckError(void* pRsp, CThostFtdcRspInfoField* pInfo, const std::string& callbackName)
{
    if (pInfo == nullptr)
    {
        logger.error("MDCTP::CheckError,{} pInfo is nullptr", callbackName);
        return false;
    }
    else if (pInfo->ErrorID != 0)
    {
        logger.error("MDCTP::CheckError,{} ErrorID {}, ErrorMsg {}", callbackName, pInfo->ErrorID, StringUtils::GBKToUTF8(pInfo->ErrorMsg));
        return false;
    }
    else if (pRsp == nullptr)
    {
        logger.error("MDCTP::CheckError,{} pInfo ErrorID {}, ErrorMsg {}, pRsp is nullptr", callbackName, pInfo->ErrorID, StringUtils::GBKToUTF8(pInfo->ErrorMsg));
        return false;
    }
    else return true;

}


};
