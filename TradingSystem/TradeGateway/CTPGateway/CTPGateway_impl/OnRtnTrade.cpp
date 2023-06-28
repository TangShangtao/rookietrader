#include "../CTPGateway.h"
#include "Defines/TradeDefs/TradeInfo.hpp"
#include "tools/Logger.h"
void CTPGateway::OnRtnTrade(CThostFtdcTradeField *pTrade) 
{
    Logger::info("OnRtnTrade: new trade updated.\norder info: {}, \nprice: {}, \nvolume: {}", pTrade->OrderRef, pTrade->Price, pTrade->Volume);
    TradeInfo* tradeInfo = tradeInfo_to_my(pTrade);
    // if (tradeInfo)
    // {
    //     if (m_sink) m_sink->on_trade(tradeInfo);
    //     tradeInfo->release();
    // }
    tradeInfo->release();

}