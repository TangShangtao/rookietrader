#pragma once
#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include "tools/utils.h"
#include "spdlog/fmt/fmt.h"
#include "magic_enum/magic_enum.hpp"

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
    Trade
};

// remote procedure call in trading, managed by req-rsp pattern
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
    // 10:10:10.100
    char generateTime[15];

    EventHeader(uint32_t rpcID, EventType eType)
        :   rpcID(rpcID), event(eType)
    {
        std::memset(tradingDay, 0, sizeof(tradingDay));
        std::memset(generateTime, 0, sizeof(generateTime));
        std::strncpy(tradingDay, TimeUtils::GetCurrTradingDay().c_str(), sizeof(tradingDay)-1);
        std::strncpy(generateTime, TimeUtils::GetTimeNow().c_str(), sizeof(generateTime)-1);
    }

    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};{};{};{}",
            rpcID, magic_enum::enum_name(event),tradingDay,generateTime
        );
    }
};
// Event Data
struct MDReady : public EventHeader
{
    MDReady(uint32_t rpcID):EventHeader(rpcID, EventType::MDReady) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};",
            EventHeader::DebugInfo()
        );
    }
};

struct Tick : public EventHeader
{
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
        return fmt::format
        (
            "{};{};{}",
            EventHeader::DebugInfo(),instrumentID,magic_enum::enum_name(exchangeID),lastPrice
        );
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

struct RPCReqHeader
{
    uint32_t rpcID;
    RPCType rpc;
    // 20200101
    char tradingDay[9];
    // 10:10:10.100
    char generateTime[15];

    RPCReqHeader(uint32_t rpcID, RPCType rpcType)
        :   rpcID(rpcID), rpc(rpcType)
    {
        std::memset(tradingDay, 0, sizeof(tradingDay));
        std::memset(generateTime, 0, sizeof(generateTime));
        std::strncpy(tradingDay, TimeUtils::GetCurrTradingDay().c_str(), sizeof(tradingDay)-1);
        std::strncpy(generateTime, TimeUtils::GetTimeNow().c_str(), sizeof(generateTime)-1);
    }

    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};{};{};{}",
            rpcID, magic_enum::enum_name(rpc), tradingDay, generateTime
        );
    }
};

struct RPCRspHeader
{
    uint32_t rpcID;
    RPCType rpc;
    // 20200101
    char tradingDay[9];
    // 10:10:10.100
    char generateTime[15];
    bool isSucc;
    char errorMsg[32];

    RPCRspHeader(uint32_t rpcID, RPCType rpcType, bool isSucc, const std::string& msg)
        :   rpcID(rpcID), rpc(rpcType), isSucc(isSucc)
    {
        std::strncpy(tradingDay, TimeUtils::GetCurrTradingDay().c_str(), sizeof(tradingDay)-1);
        std::strncpy(generateTime, TimeUtils::GetTimeNow().c_str(), sizeof(generateTime)-1);
        std::strncpy(errorMsg, msg.c_str(), sizeof(errorMsg)-1);
    }

    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};{};{};{}",
            rpcID, magic_enum::enum_name(rpc), tradingDay, generateTime
        );
    }
};

struct PrepareMDReq : public RPCReqHeader
{
    PrepareMDReq(uint32_t rpcID):RPCReqHeader(rpcID, RPCType::PrepareMD) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{}",
            RPCReqHeader::DebugInfo()
        );
    }
};

struct PrepareMDRsp : public RPCRspHeader
{
    PrepareMDRsp(uint32_t rpcID, bool isSucc, const std::string& msg):RPCRspHeader(rpcID, RPCType::PrepareMD, isSucc, msg) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};{};{}",
            RPCRspHeader::DebugInfo(),isSucc,errorMsg
        );
    }
};

struct PrepareTDReq : public RPCReqHeader
{
    PrepareTDReq(uint32_t rpcID):RPCReqHeader(rpcID, RPCType::PrepareTD) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};",
            RPCReqHeader::DebugInfo()
        );
    }
};

struct PrepareTDRsp : public RPCRspHeader
{
    PrepareTDRsp(uint32_t rpcID, bool isError, const std::string& msg):RPCRspHeader(rpcID, RPCType::PrepareTD, isError, msg) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};",
            RPCRspHeader::DebugInfo()
        );
    }
};

struct QryInstrumentsReq : public RPCReqHeader
{
    QryInstrumentsReq(uint32_t rpcID):RPCReqHeader(rpcID, RPCType::QryInstruments) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};",
            RPCReqHeader::DebugInfo()
        );
    }
};

struct QryInstrumentsRsp : public RPCRspHeader
{
    QryInstrumentsRsp(uint32_t rpcID, bool isError, const std::string& msg):RPCRspHeader(rpcID, RPCType::QryInstruments, isError, msg) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};",
            RPCRspHeader::DebugInfo()
        );
    }
};

struct QryPositionReq : public RPCReqHeader
{
    QryPositionReq(uint32_t rpcID):RPCReqHeader(rpcID, RPCType::QryPosition) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};",
            RPCReqHeader::DebugInfo()
        );
    }
};

struct QryPositionRsp : public RPCRspHeader
{
    QryPositionRsp(uint32_t rpcID, bool isError, const std::string& msg):RPCRspHeader(rpcID, RPCType::QryPosition, isError, msg) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};",
            RPCRspHeader::DebugInfo()
        );
    }
};

struct QryAccountReq : public RPCReqHeader
{
    QryAccountReq(uint32_t rpcID):RPCReqHeader(rpcID, RPCType::QryAccount) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};",
            RPCReqHeader::DebugInfo()
        );
    }
};

struct QryAccountRsp : public RPCRspHeader
{
    QryAccountRsp(uint32_t rpcID, bool isError, const std::string& msg):RPCRspHeader(rpcID, RPCType::QryAccount, isError, msg) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};",
            RPCRspHeader::DebugInfo()
        );
    }
};

struct SubTickReq : public RPCReqHeader
{
    static const uint16_t instrumentIDLen = 16;
    ExchangeID exchange;
    uint64_t counts;
    // total byte len of this struct
    uint64_t byteSize;
    SubTickReq(uint32_t rpcID, ExchangeID exchange, uint64_t counts)
        :   RPCReqHeader(rpcID, RPCType::SubTick),
            exchange(exchange), counts(counts)
    {
        byteSize = sizeof(SubTickReq) + instrumentIDLen * counts;
    }

    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};{};{};",
            RPCReqHeader::DebugInfo(), magic_enum::enum_name(exchange), counts
        );
    }
};

struct SubTickRsp : public RPCRspHeader
{
    int errorID;
    char errorMsg[32];
    SubTickRsp(uint32_t rpcID, bool isError, const std::string& msg):RPCRspHeader(rpcID, RPCType::SubTick, isError, msg) {}

    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};",
            RPCRspHeader::DebugInfo()
        );
    }
};

struct SubBarReq : public RPCReqHeader
{
    char** instrumentIDs;
    uint64_t counts;
    const uint16_t instrumentIDLen = 16;
    SubBarReq(uint32_t rpcID):RPCReqHeader(rpcID, RPCType::SubBar) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct SubBarRsp : public RPCRspHeader
{
    SubBarRsp(uint32_t rpcID, bool isError, const std::string& msg):RPCRspHeader(rpcID, RPCType::SubBar, isError, msg) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct OrderInsertReq : public RPCReqHeader
{
    uint64_t orderReqID;
    OrderInsertReq(uint32_t rpcID):RPCReqHeader(rpcID, RPCType::OrderInsert) {}
    std::string DebugInfo() const
    {
        return "";
    }

};

struct OrderInsertRsp : public RPCRspHeader
{
    uint64_t orderReqID;
    OrderInsertRsp(uint32_t rpcID, bool isError, const std::string& msg):RPCRspHeader(rpcID, RPCType::OrderInsert, isError, msg) {}
    std::string DebugInfo() const
    {
        return "";
    }

};

struct CancelReq : public RPCReqHeader
{
    uint64_t cancelReqID;
    uint64_t orderReqID;
    CancelReq(uint32_t rpcID):RPCReqHeader(rpcID, RPCType::Cancel) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct CancelRsp : public RPCRspHeader
{
    uint64_t cancelReqID;
    uint64_t orderReqID;
    CancelRsp(uint32_t rpcID, bool isError, const std::string& msg):RPCRspHeader(rpcID, RPCType::Cancel, isError, msg) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

};


