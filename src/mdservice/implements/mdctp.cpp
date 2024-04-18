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
    logger.info("MDCTP::OnPrepareMDReq,called");
    return true;

}   
bool MDCTP::OnSubTickReq(const SubTickReq* subTickReq) 
{
    return true;
}

void MDCTP::OnFrontConnected() 
{
    logger.info("MDCTP::OnFrontConnected,call ReqUserLogin; reqID {}", prepareMDRpcID);
    ctpMdApi->ReqUserLogin(nullptr, prepareMDRpcID);
    logger.info("MDCTP::OnFrontConnected,called");
}

void MDCTP::OnFrontDisconnected(int nReason) 
{
    logger.error("MDCTP::OnFrontDisconnected,called; nReason {}", nReason);
}

void MDCTP::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{

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

};
