#pragma once
#include "enum.h"
#include <array>
#include "spdlog/fmt/fmt.h"
#include "magic_enum/magic_enum.hpp"

namespace rookietrader
{

struct Request 
{
    RPCType type;
    std::array<char, 9> trading_day;    // %Y%m%d
    std::array<char, 10> request_time;  // %H%M%S
    const std::string trading_day_string() const { return trading_day.data(); }
    const std::string request_time_string() const { return trading_day.data(); }

    Request(RPCType type) : type(type) 
    {
        std::memset(trading_day.data(), 0, sizeof(trading_day));
        std::memset(request_time.data(), 0, sizeof(request_time));        
    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};{};{}",
            magic_enum::enum_name(type), trading_day.data(), request_time.data()
        );
    }

};

struct Response 
{
    RPCType type;
    std::array<char, 9> trading_day;    // %Y%m%d
    std::array<char, 10> request_time;  // %H%M%S
    std::array<char, 10> response_time; // %H%M%S
    bool result;
    const std::string trading_day_string() const { return trading_day.data(); }
    const std::string request_time_string() const { return trading_day.data(); }
    const std::string response_time_string() const { return response_time.data(); }
    Response(RPCType type) : type(type) 
    {
        std::memset(trading_day.data(), 0, sizeof(trading_day));
        std::memset(request_time.data(), 0, sizeof(request_time));   
        std::memset(response_time.data(), 0, sizeof(response_time));       
    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};{};{};{};{}",
            magic_enum::enum_name(type), trading_day.data(), request_time.data(), response_time.data(), result
        );
    }
};

struct ConnectMarketDataRequest : public Request 
{
    ConnectMarketDataRequest() : Request(RPCType::RTConnectMarketData) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Request::DebugInfo()
        );
    }
};

struct ConnectMarketDataResponse : public Response 
{
    ConnectMarketDataResponse() : Response(RPCType::RTConnectMarketData) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Response::DebugInfo()
        );
    }
};

struct DisconnectMarketDataRequest : public Request 
{
    DisconnectMarketDataRequest() : Request(RPCType::RTDisconnectMarketData) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Request::DebugInfo()
        );
    }
};

struct DisconnectMarketDataResponse : public Response 
{
    DisconnectMarketDataResponse() : Response(RPCType::RTDisconnectMarketData) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Response::DebugInfo()
        );
    }
};

struct QuerySymbolRequest : public Request 
{
    QuerySymbolRequest() : Request(RPCType::RTQuerySymbol) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Request::DebugInfo()
        );
    }
};

struct QuerySymbolResponse : public Response 
{
    QuerySymbolResponse() : Response(RPCType::RTQuerySymbol) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Response::DebugInfo()
        );
    }
};

struct SubscribeSymbolRequest : public Request 
{
    SubscribeSymbolRequest() : Request(RPCType::RTSubscribeSymbol) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Request::DebugInfo()
        );
    }
};

struct SubscribeSymbolResponse : public Response 
{
    SubscribeSymbolResponse() : Response(RPCType::RTSubscribeSymbol) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Response::DebugInfo()
        );
    }
};

struct UnsubscribeSymbolRequest : public Request 
{
    UnsubscribeSymbolRequest() : Request(RPCType::RTUnsubscribeSymbol) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Request::DebugInfo()
        );
    }
};

struct UnsubscribeSymbolResponse : public Response 
{
    UnsubscribeSymbolResponse() : Response(RPCType::RTUnsubscribeSymbol) 
    {

    }

    std::string DebugInfo() const 
    {
        return fmt::format(
            "{};",
            Response::DebugInfo()
        );
    }
};

};