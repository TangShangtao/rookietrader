// mdservice interface implementation of CTP 
#pragma once
#include "nlohmann/json.hpp"
#include "CTP6.3.15/ThostFtdcMdApi.h"
#include "../mdservice.h"

namespace rookietrader
{

class MDCTP : public CThostFtdcMdSpi, public MDService
{
public:
    MDCTP(
        const std::string& eventUrl, 
        const std::string& rpcUrl, 
        const std::string& accountID, 
        const std::string& password,
        const std::string& frontAddr,
        const std::string& loggerName,
        const std::string& logMode
    ); 
    ~MDCTP() override;
    bool OnPrepareMDReq(const PrepareMDReq* req) override;
    bool OnSubTickReq(const SubTickReq* subTickReq) override;

    void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;
private:
    bool CheckError(void* pRsp, CThostFtdcRspInfoField* pInfo, const std::string& callbackName);
    
private:
    CThostFtdcMdApi* ctpMdApi;
};

};
