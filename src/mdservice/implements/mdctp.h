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
    MDCTP(const nlohmann::json& config);
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
    CThostFtdcMdApi* ctpMdApi;
};

};