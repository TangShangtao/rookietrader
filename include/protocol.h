#pragma once
#include <cstdint>
#include <string>
#include <tools/utils.h>


namespace rookietrader
{

// Events in trading, managed by pub-sub pattern
enum class Event
{
    // MDService ready to communicate
    MDReady,
    // TDService ready to communicate
    TDReady,
    // marketdata report tick
    Tick,
    // trader report order
    Order,
    // trader report trade
    Trade,
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
    // Tick
    Event event;
    // 20200101
    char tradingDay[9];
    // 10:10:10
    char generateTime[9];
    // 100
    uint32_t generateMillisec;

    EventHeader(Event eType)
        :   event(eType)
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
    MDReady():EventHeader(Event::MDReady) {}
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
    Tick():EventHeader(Event::Tick) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

// struct Bar
// {
//     EventHeader header;
//     std::string DebugInfo()
//     {

//     }
// };

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
    Trade():EventHeader(Event::Trade) {}
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
    Order():EventHeader(Event::Order) {}
    std::string DebugInfo() const
    {
        return "";
    }
};

struct Position : public EventHeader
{
    char accountID[32]; 
    Position():EventHeader(Event::Position) {}
    std::string DebugInfo() const
    {
        return "";
    } 
};

struct Account : public EventHeader
{
    char accountID[32];   
    std::string DebugInfo() const
    {
        return "";
    }
};

// Requests
struct SubscribeMarketDataReq : public EventHeader
{
    char** instrumentIDs;
    uint64_t counts;
    const uint16_t instrumentIDLen = 16;
    std::string DebugInfo() const
    {
        return "";
    }
};

struct OrderReq : public EventHeader
{
    uint64_t orderReqID;
    std::string DebugInfo() const
    {
        return "";
    }

};

struct CancelReq : public EventHeader
{
    uint64_t cancelReqID;
    uint64_t orderReqID;
    std::string DebugInfo() const
    {
        return "";
    }
};

struct QryPositionReq : public EventHeader
{
    char accountID[32];   
    std::string DebugInfo() const
    {
        return "";
    }
};

struct QryAccountReq : public EventHeader
{
    char accountID[32];   
    std::string DebugInfo() const
    {
        return "";
    }
};

};


