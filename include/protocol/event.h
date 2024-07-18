#pragma once
#include "enum.h"
#include <array>
#include "spdlog/fmt/fmt.h"
#include "magic_enum/magic_enum.hpp"

namespace rookietrader 
{

struct Event
{
    EventType type;
    std::array<char,9> trading_day;     // %Y%m%d
    std::array<char,10> generate_time;  // %H%M%S
    const std::string trading_day_string() const { return trading_day.data(); }
    const std::string generate_time_string() const { return generate_time.data(); }

    Event(EventType type) : type(type) 
    {
        std::memset(trading_day.data(), 0, sizeof(trading_day));
        std::memset(generate_time.data(), 0, sizeof(generate_time));
    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};{};{}",
            magic_enum::enum_name(type),
            trading_day.data(),
            generate_time.data()
        );
    }
};

struct MarketDataDisconnected : public Event 
{
    MarketDataDisconnected() : Event(EventType::ETMarketDataDisconnected) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Event::DebugInfo()
        );
    }
};

struct MarketDataConnected : public Event 
{
    MarketDataConnected() : Event(EventType::ETMarketDataConnected) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Event::DebugInfo()
        );
    }
};

struct TradeDisconnected : public Event 
{
    TradeDisconnected() : Event(EventType::ETTradeDisconnected) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Event::DebugInfo()
        );
    }
};

struct TradeConnected : public Event 
{
    TradeConnected() : Event(EventType::ETTradeConnected) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Event::DebugInfo()
        );
    }
};

struct TopOfBookSnapshotData : public Event 
{
    std::array<char,16> symbol;
    std::array<char, 10> update_time;
    ExchangeID exchange_id;
    uint64_t min_inverval;

    double last_price;
    double last_volume;

    double bid_price1;
    double ask_price1;
    uint32_t bid_volume1;
    uint32_t ask_volume1;
    const std::string symbol_string() const {return symbol.data(); }
    const std::string update_time_string() const {return update_time.data(); }


    TopOfBookSnapshotData() : Event(EventType::ETTopOfBookSnapshotData) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};"
            "{};{};{};{};"
            "{};{};{};{};{};{};",
            Event::DebugInfo(),
            symbol.data(), update_time.data(), magic_enum::enum_name(exchange_id), min_inverval, 
            last_price, last_volume, bid_price1, ask_price1, bid_volume1, ask_volume1
        );
    }
};
};