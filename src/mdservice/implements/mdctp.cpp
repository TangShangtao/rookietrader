#include "mdctp.h"
namespace rookietrader
{
MDCTP::MDCTP(nlohmann::json config)
    :   MDService(config),
        mdApi(CThostFtdcMdApi::CreateFtdcMdApi())
{
    logger.debug("MDCTP::MDCTP,called");
    std::string frontAddrCopy = frontAddr;
    mdApi->RegisterFront(frontAddrCopy.data());
    mdApi->RegisterSpi(this);
}

MDCTP::~MDCTP()
{
    mdApi->Join();
    mdApi->Release();
    logger.debug("MDCTP::~MDCTP,called");
}

bool MDCTP::Prepare()
{   
    mdApi->Init();
    logger.info("MDCTP::Prepare,called");
    return true;

}   
bool MDCTP::SubscribeMarketData(SubscribeMarketDataReq instrumentIDs)
{

}

void MDCTP::OnFrontConnected()
{
    logger.info("MDCTP::OnFrontConnected,call ReqUserLogin reqID {}", ++reqID);
    mdApi->ReqUserLogin(nullptr, reqID);
    logger.info("MDCTP::OnFrontConnected,called");
}

void MDCTP::OnFrontDisconnected(int nReason) 
{
    logger.error("MDCTP::OnFrontDisconnected,called nReason {}", nReason);
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
