#include "mdctp.h"
namespace rookietrader
{
MDCTP::MDCTP(
    const std::string& eventUrl, 
    const std::string& rpcUrl, 
    const std::string& accountID, 
    const std::string& password,
    const std::string& frontAddr,
    const std::string& loggerName,
    const std::string& logMode
)   :   
    MDService(eventUrl,rpcUrl,accountID,password,frontAddr,loggerName,logMode),
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
    logger.debug("MDCTP::OnPrepareMDReq,ctpMdApi->Init() called");
    return true;

}   
bool MDCTP::OnDisconnect()
{
    return false;
}
bool MDCTP::OnSubTickReq(const SubTickReq* subTickReq) 
{
    char* pInstruments = (char*)subTickReq;
    pInstruments += sizeof(SubTickReq);
    char** ppInstrumentID = (char**)std::malloc(subTickReq->counts * sizeof(char*));
    for (int i = 0; i < subTickReq->counts; ++i)
    {
        ppInstrumentID[i] = pInstruments;
        pInstruments += SubTickReq::instrumentIDLen;        
    }
    int res = ctpMdApi->SubscribeMarketData(ppInstrumentID, subTickReq->counts);
    if (res != 0)
    {
        logger.error("MDCTP::OnSubTickReq,ctpMdApi->SubscribeMarketData() return res {}, SubTick {}", res, subTickReq->DebugInfo());
        SendSubTickRsp(false, "ctpMdApi->SubscribeMarketData() return res != 0");
        return false;
    }
    logger.debug("MDCTP::OnSubTickReq,ctpMdApi->SubscribeMarketData() called");
    return true;
}

void MDCTP::OnFrontConnected() 
{
    CThostFtdcReqUserLoginField login;
    std::memset(&login, 0, sizeof(login));
    std::memcpy(login.UserID, accountID.c_str(), sizeof(login.UserID)-1);
    std::memcpy(login.Password, password.c_str(), sizeof(login.Password)-1);
    std::memcpy(login.BrokerID, "9999", sizeof(login.BrokerID)-1);
    ctpMdApi->ReqUserLogin(&login, prepareMDRpcID);
    logger.debug("MDCTP::OnFrontConnected,call ReqUserLogin; reqID {}", prepareMDRpcID);
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
    }
    SendPrepareMDRsp(true, "");
    PublishMDReady();
    logger.debug("MDCTP::OnRspUserLogin, MDCTP prepared!");
}

void MDCTP::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{

}

void MDCTP::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    if (CheckError(pSpecificInstrument, pRspInfo, "OnRspSubMarketData") == false)
    {
        SendSubTickRsp(false, "CTP OnRspSubMarketData CheckError");
        logger.error("MDCTP::OnRspSubMarketData, MDCTP failed to subscribe");
        exit(-1);
    }
    if (bIsLast)
    {
        SendSubTickRsp(true, "");
        logger.debug("MDCTP::OnRspSubMarketData, called");
    }
    
}

void MDCTP::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) 
{
    if (pDepthMarketData == nullptr)
    {
        logger.error("MDCTP::OnRtnDepthMarketData pDepthMarketData is nullptr !");
        exit(-1);
    }
    logger.debug("MDCTP::OnRtnDepthMarketData {}, {}, {}", pDepthMarketData->InstrumentID, pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec);
    Tick tick(subTickRpcID);
    std::string updateTime = pDepthMarketData->UpdateTime;
    updateTime += ".";
    updateTime += std::to_string(pDepthMarketData->UpdateMillisec);
    std::memcpy(tick.updateTime.data(), updateTime.c_str(), sizeof(tick.updateTime));
    std::memcpy(tick.instrumentID.data(), pDepthMarketData->InstrumentID, sizeof(tick.instrumentID)-1);
    if (std::strcmp(pDepthMarketData->ExchangeID, magic_enum::enum_name(ExchangeID::SHFE).data()) == 0)
    {
        tick.exchangeID = ExchangeID::SHFE;
    }
    else if (std::strcmp(pDepthMarketData->ExchangeID, magic_enum::enum_name(ExchangeID::DCE).data()) == 0)
    {
        tick.exchangeID = ExchangeID::DCE;
    }
    else if (std::strcmp(pDepthMarketData->ExchangeID, magic_enum::enum_name(ExchangeID::CZCE).data()) == 0)
    {
        tick.exchangeID = ExchangeID::CZCE;
    }
    else if (std::strcmp(pDepthMarketData->ExchangeID, magic_enum::enum_name(ExchangeID::CFFEX).data()) == 0)
    {
        tick.exchangeID = ExchangeID::CFFEX;
    }
    else {tick.exchangeID == ExchangeID::ExchangeIDNone;}
    tick.lastPrice = pDepthMarketData->LastPrice;
    tick.bidPrices[0] = pDepthMarketData->BidPrice1;
    tick.bidPrices[1] = pDepthMarketData->BidPrice2;
    tick.bidPrices[2] = pDepthMarketData->BidPrice3;
    tick.bidPrices[3] = pDepthMarketData->BidPrice4;
    tick.bidPrices[4] = pDepthMarketData->BidPrice5;

    tick.bidVolumes[0] = pDepthMarketData->BidVolume1;
    tick.bidVolumes[1] = pDepthMarketData->BidVolume2;
    tick.bidVolumes[2] = pDepthMarketData->BidVolume3;
    tick.bidVolumes[3] = pDepthMarketData->BidVolume4;
    tick.bidVolumes[4] = pDepthMarketData->BidVolume5;

    tick.askPrices[0] = pDepthMarketData->AskPrice1;
    tick.askPrices[1] = pDepthMarketData->AskPrice2;
    tick.askPrices[2] = pDepthMarketData->AskPrice3;
    tick.askPrices[3] = pDepthMarketData->AskPrice4;
    tick.askPrices[4] = pDepthMarketData->AskPrice5;

    tick.askVolumes[0] = pDepthMarketData->AskVolume1;
    tick.askVolumes[1] = pDepthMarketData->AskVolume2;
    tick.askVolumes[2] = pDepthMarketData->AskVolume3;
    tick.askVolumes[3] = pDepthMarketData->AskVolume4;
    tick.askVolumes[4] = pDepthMarketData->AskVolume5;

    PublishTick(tick);
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
