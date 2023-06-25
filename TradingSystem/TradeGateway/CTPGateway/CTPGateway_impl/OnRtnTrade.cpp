#include "../CTPGateway.h"
#include "Defines/TradeDefs/TradeInfo.hpp"
#include "tools/Logger.h"
void CTPGateway::OnRtnTrade(CThostFtdcTradeField *pTrade) 
{
    Logger::info("OnRtnTrade !!");
    TradeInfo* tradeInfo = tradeInfo_to_my(pTrade);
    // if (tradeInfo)
    // {
    //     if (m_sink) m_sink->on_trade(tradeInfo);
    //     tradeInfo->release();
    // }
    tradeInfo->release();

}