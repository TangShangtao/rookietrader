#pragma once
#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <array>
#include <vector>

#include "tools/utils.h"
#include "spdlog/fmt/fmt.h"
#include "magic_enum/magic_enum.hpp"

namespace rookietrader
{
// Events in trading, managed by pub-sub pattern
enum class EventType
{
    EventNone,
    // MDService ready to communicate
    EventMDReady,
    // TDService ready to communicate
    EventTDReady,
    // marketdata report tick
    EventTick,
    // marketdata report bar
    EventBar,
    // trader report order
    EventOrder,
    // trader report trade
    EventTrade
};

// remote procedure call in trading, managed by req-rsp pattern
enum class RPCType
{
    None,
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
    None,
    SHFE,
    DCE,
    CZCE,
    CFFEX,
    
};

enum class OrderDirection
{
    None,
    Long, 
    Short
};

enum class Offset
{
    None,
    Open,
    CloseTd,
    CloseYd,
};

enum class OrderStatus
{
    None,
    // RiskControlled,
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
    std::array<char, 9> tradingDay;
    // 10:10:10.100
    std::array<char, 15> generateTime;
    const std::string GetTradingDay() const {return tradingDay.data();}
    const std::string GetGenerateTime() const {return generateTime.data();}
    EventHeader(uint32_t rpcID, EventType eType)
        :   rpcID(rpcID), event(eType)
    {
        std::memset(tradingDay.data(), 0, sizeof(tradingDay));
        std::memset(generateTime.data(), 0, sizeof(generateTime));
        std::memcpy(tradingDay.data(), TimeUtils::GetCurrTradingDay().c_str(), sizeof(tradingDay)-1);
        std::memcpy(generateTime.data(), TimeUtils::GetTimeNow().c_str(), sizeof(generateTime)-1);
    }

    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};{};{};{}",
            rpcID, magic_enum::enum_name(event),tradingDay.data(),generateTime.data()
        );
    }
};
// Event Data
struct MDReady : public EventHeader
{
    MDReady(uint32_t rpcID):EventHeader(rpcID, EventType::EventMDReady) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};",
            EventHeader::DebugInfo()
        );
    }
};
struct TDReady : public EventHeader
{
    TDReady(uint32_t rpcID):EventHeader(rpcID, EventType::EventTDReady) {}
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
    // format: 10:10:10.100
    std::array<char, 15> updateTime;
    std::array<char, 16> instrumentID;
    ExchangeID exchangeID;
    double lastPrice;
    std::array<double, 5> bidPrices;
    std::array<double, 5> bidVolumes;
    std::array<double, 5> askPrices;
    std::array<double, 5> askVolumes;
    const std::string GetUpdateTime() const {return updateTime.data();}
    const std::string GetInstrumentID() const {return updateTime.data();}
    Tick(uint32_t rpcID):EventHeader(rpcID, EventType::EventTick) {}
    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};{};{}",
            EventHeader::DebugInfo(),instrumentID.data(),magic_enum::enum_name(exchangeID),lastPrice
        );
    }
};

struct Bar : public EventHeader
{
    Bar(uint32_t rpcID):EventHeader(rpcID, EventType::EventBar) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct Trade : public EventHeader
{
    std::array<char, 32> tradeID;
    uint64_t orderReqID;
    std::array<char, 32> orderSysID;
    std::array<char, 32> accountID;   

    std::array<char, 16> instrumentID;
    ExchangeID exchangeID;
    double tradePrice;
    double orderPrice;
    // traded volume in on trade
    int tradeVolume;
    // origin total volume
    int orderVolume;
    double commission;
    Trade(uint32_t rpcID):EventHeader(rpcID, EventType::EventTrade) {}
    std::string DebugInfo() const
    {
        return "";
    }

};

struct Order : public EventHeader
{
    uint64_t orderReqID;
    std::array<char, 32> orderSysID;
    std::array<char, 32> accountID;   

    std::array<char, 16> instrumentID;
    ExchangeID exchangeID;
    OrderDirection orderDirection;
    Offset offset;
    double limitPrice;
    uint32_t totalVolume;

    uint32_t tradedVolume;
    OrderStatus orderStatus;
    Order(uint32_t rpcID):EventHeader(rpcID, EventType::EventOrder) {}
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
    std::array<char, 9> tradingDay;
    // 10:10:10.100
    std::array<char, 15> generateTime;

    RPCReqHeader(uint32_t rpcID, RPCType rpcType)
        :   rpcID(rpcID), rpc(rpcType)
    {
        std::memset(tradingDay.data(), 0, sizeof(tradingDay));
        std::memset(generateTime.data(), 0, sizeof(generateTime));
        std::memcpy(tradingDay.data(), TimeUtils::GetCurrTradingDay().c_str(), sizeof(tradingDay)-1);
        std::memcpy(generateTime.data(), TimeUtils::GetTimeNow().c_str(), sizeof(generateTime)-1);
    }

    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};{};{};{}",
            rpcID, magic_enum::enum_name(rpc), tradingDay.data(), generateTime.data()
        );
    }
};

struct RPCRspHeader
{
    uint32_t rpcID;
    RPCType rpc;
    // 20200101
    std::array<char, 9> tradingDay;
    // 10:10:10.100
    std::array<char, 15> generateTime;
    bool isSucc;
    std::array<char, 32> errorMsg;

    RPCRspHeader(uint32_t rpcID, RPCType rpcType, bool isSucc, const std::string& msg)
        :   rpcID(rpcID), rpc(rpcType), isSucc(isSucc)
    {
        std::memcpy(tradingDay.data(), TimeUtils::GetCurrTradingDay().c_str(), sizeof(tradingDay)-1);
        std::memcpy(generateTime.data(), TimeUtils::GetTimeNow().c_str(), sizeof(generateTime)-1);
        std::memcpy(errorMsg.data(), msg.c_str(), sizeof(errorMsg)-1);
    }

    std::string DebugInfo() const
    {
        return fmt::format
        (
            "{};{};{};{}",
            rpcID, magic_enum::enum_name(rpc), tradingDay.data(), generateTime.data()
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
            RPCRspHeader::DebugInfo(),isSucc,errorMsg.data()
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

// consist byte of instruments followed by SubTickReq
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
    std::array<char, 32> errorMsg;
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


