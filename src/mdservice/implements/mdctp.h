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
    MDCTP(nlohmann::json config);
    ~MDCTP() override;
    bool Prepare() noexcept override;
    bool SubscribeMarketData(SubTickReq subReq) noexcept override;

    void OnFrontConnected() noexcept override;
    void OnFrontDisconnected(int nReason) noexcept override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept override;
    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept override;
	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept override;
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) noexcept override;
private:
    CThostFtdcMdApi* mdApi;
};

};
