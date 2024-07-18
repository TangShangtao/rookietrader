#pragma once

namespace rookietrader
{

enum class EventType
{
    ETNone,

    ETMarketDataDisconnected,
    ETMarketDataConnected,
    ETTopOfBookSnapshotData,
    ETBarData,

    ETTradeDisconnected,
    ETTradeConnected,
    ETOrderData,
    ETTradeData,
};

enum class RPCType
{
    RTNone,

    RTConnectMarketData,
    RTDisconnectMarketData,
    RTQuerySymbol,
    RTSubscribeSymbol,
    RTUnsubscribeSymbol,
    
    RTConnectTrade,
    RTDisconnectTrade,
    RTQueryAccount,
    RTQueryPosition,
    RTPlaceOrder,
    RTCancelOrder,
};

enum class ExchangeID
{
    ExchangeIDNone,
    NYMEX,
    COMEX,
    CBOT,
    CME,
};



};