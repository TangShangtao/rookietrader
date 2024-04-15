#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include "tools/utils.h"


namespace rookietrader
{
// Events in trading, managed by pub-sub pattern
enum class EventType
{
    // MDService ready to communicate
    MDReady,
    // TDService ready to communicate
    TDReady,
    // marketdata report tick
    Tick,
    // marketdata report bar
    Bar,
    // trader report order
    Order,
    // trader report trade
    Trade,
};
// remote procedure call in trading, managed by req-rsp pattern
// rsp called only on error cases
enum class RPCType
{
    PrepareMD,
    PrepareTD,
    QryInstruments,
    QryPosition,
    QryAccount,
    SubTick,
    SubBar,
    OrderInsert,
    Cancel,

};

enum class ExchangeID
{
    SHFE
};

enum class OrderDirection
{
    Long, 
    Short
};

enum class Offset
{
    Open,
    CloseTd,
    CloseYd,
};

enum class OrderStatus
{
    RiskControlled,
    Rejected,
    NoTradedQueueing,
    NoTradedCancelled,
    PartlyTradedQueueing,
    PartlyTradedCancelled,
    AllTraded

};
struct EventHeader
{
    uint32_t rpcID;
    // Tick
    EventType event;
    // 20200101
    char tradingDay[9];
    // 10:10:10
    char generateTime[9];
    // 100
    uint32_t generateMillisec;

    EventHeader(uint32_t rpcID, EventType eType)
        :   rpcID(rpcID), event(eType)
    {
        std::strncpy(tradingDay, TimeUtils::GetCurrTradingDay().c_str(), sizeof(tradingDay)-1);
        std::strncpy(generateTime, TimeUtils::GetTimeNow().c_str(), sizeof(generateTime)-1);
        generateMillisec = TimeUtils::GetCurrMillisec();
    }

    std::string DebugInfo() const
    {
        return "";
    }
};
// Event Data
struct MDReady : public EventHeader
{
    char url[16];
    MDReady(uint32_t rpcID):EventHeader(rpcID, EventType::MDReady) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct Tick : public EventHeader
{
    char url[16];
    char instrumentID[16];
    ExchangeID exchangeID;
    double lastPrice;
    double bidPrices[5];
    double bidVolumes[5];
    double askPrices[5];
    double askVolumes[5];
    Tick(uint32_t rpcID):EventHeader(rpcID, EventType::Tick) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct Bar : public EventHeader
{
    Bar(uint32_t rpcID):EventHeader(rpcID, EventType::Bar) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct Trade : public EventHeader
{
    char tradeID[32];
    uint64_t orderReqID;
    char orderSysID[32];
    char accountID[32];   

    char instrumentID[16];
    ExchangeID exchangeID;
    double tradePrice;
    double orderPrice;
    // traded volume in on trade
    int tradeVolume;
    // origin total volume
    int orderVolume;
    double commission;
    Trade(uint32_t rpcID):EventHeader(rpcID, EventType::Trade) {}
    std::string DebugInfo() const
    {
        return "";
    }

};

struct Order : public EventHeader
{
    uint64_t orderReqID;
    char orderSysID[32];
    char accountID[32];   

    char instrumentID[16];
    ExchangeID exchangeID;
    OrderDirection orderDirection;
    Offset offset;
    double limitPrice;
    uint32_t totalVolume;

    uint32_t tradedVolume;
    OrderStatus orderStatus;
    Order(uint32_t rpcID):EventHeader(rpcID, EventType::Order) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct RPCHeader
{
    uint32_t rpcID;
    RPCType rpc;
    // 20200101
    char tradingDay[9];
    // 10:10:10
    char generateTime[9];
    // 100
    uint32_t generateMillisec;

    RPCHeader(uint32_t rpcID, RPCType rpcType)
        :   rpcID(rpcID), rpc(rpcType)
    {
        std::strncpy(tradingDay, TimeUtils::GetCurrTradingDay().c_str(), sizeof(tradingDay)-1);
        std::strncpy(generateTime, TimeUtils::GetTimeNow().c_str(), sizeof(generateTime)-1);
        generateMillisec = TimeUtils::GetCurrMillisec();
    }

    std::string DebugInfo() const
    {
        return "";
    }
};

struct PrepareMDReq : public RPCHeader
{
    PrepareMDReq(uint32_t rpcID):RPCHeader(rpcID, RPCType::PrepareMD) {}
    std::string DebugInfo() const
    {
        return "";
    } 
};

struct PrepareMDRsp : public RPCHeader
{
    PrepareMDRsp(uint32_t rpcID):RPCHeader(rpcID, RPCType::PrepareMD) {}
    std::string DebugInfo() const
    {
        return "";
    } 
};

struct PrepareTDReq : public RPCHeader
{
    PrepareTDReq(uint32_t rpcID):RPCHeader(rpcID, RPCType::PrepareTD) {}
    std::string DebugInfo() const
    {
        return "";
    } 
};

struct PrepareTDRsp : public RPCHeader
{
    PrepareTDRsp(uint32_t rpcID):RPCHeader(rpcID, RPCType::PrepareTD) {}
    std::string DebugInfo() const
    {
        return "";
    } 
};

struct QryInstrumentsReq : public RPCHeader
{
    QryInstrumentsReq(uint32_t rpcID):RPCHeader(rpcID, RPCType::QryInstruments) {}
    std::string DebugInfo() const
    {
        return "";
    } 
};

struct QryInstrumentsRsp : public RPCHeader
{
    QryInstrumentsRsp(uint32_t rpcID):RPCHeader(rpcID, RPCType::QryInstruments) {}
    std::string DebugInfo() const
    {
        return "";
    } 
};

struct QryPositionReq : public RPCHeader
{
    QryPositionReq(uint32_t rpcID):RPCHeader(rpcID, RPCType::QryPosition) {}
    std::string DebugInfo() const
    {
        return "";
    } 
};

struct QryPositionRsp : public RPCHeader
{
    QryPositionRsp(uint32_t rpcID):RPCHeader(rpcID, RPCType::QryPosition) {}
    std::string DebugInfo() const
    {
        return "";
    } 
};

struct QryAccountReq : public RPCHeader
{
    QryAccountReq(uint32_t rpcID):RPCHeader(rpcID, RPCType::QryAccount) {}
    std::string DebugInfo() const
    {
        return "";
    } 
};

struct QryAccountRsp : public RPCHeader
{
    QryAccountRsp(uint32_t rpcID):RPCHeader(rpcID, RPCType::QryAccount) {}
    std::string DebugInfo() const
    {
        return "";
    } 
};

struct SubTickReq : public RPCHeader
{
    char** instrumentIDs;
    ExchangeID exchange;
    uint64_t counts;
    const uint16_t instrumentIDLen = 16;
    SubTickReq(uint32_t rpcID):RPCHeader(rpcID, RPCType::SubTick) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct SubTickRsp : public RPCHeader
{
    SubTickRsp(uint32_t rpcID):RPCHeader(rpcID, RPCType::SubTick) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct SubBarReq : public RPCHeader
{
    char** instrumentIDs;
    uint64_t counts;
    const uint16_t instrumentIDLen = 16;
    SubBarReq(uint32_t rpcID):RPCHeader(rpcID, RPCType::SubBar) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct SubBarRsp : public RPCHeader
{
    SubBarRsp(uint32_t rpcID):RPCHeader(rpcID, RPCType::SubBar) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct OrderInsertReq : public RPCHeader
{
    uint64_t orderReqID;
    OrderInsertReq(uint32_t rpcID):RPCHeader(rpcID, RPCType::OrderInsert) {}
    std::string DebugInfo() const
    {
        return "";
    }

};

struct OrderInsertRsp : public RPCHeader
{
    uint64_t orderReqID;
    OrderInsertRsp(uint32_t rpcID):RPCHeader(rpcID, RPCType::OrderInsert) {}
    std::string DebugInfo() const
    {
        return "";
    }

};

struct CancelReq : public RPCHeader
{
    uint64_t cancelReqID;
    uint64_t orderReqID;
    CancelReq(uint32_t rpcID):RPCHeader(rpcID, RPCType::Cancel) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct CancelRsp : public RPCHeader
{
    uint64_t cancelReqID;
    uint64_t orderReqID;
    CancelRsp(uint32_t rpcID):RPCHeader(rpcID, RPCType::Cancel) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

};


