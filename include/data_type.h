//
// Created by root on 25-9-15.
//

#pragma once
#include <string>
#include <vector>
#include <utility>
#include <cinttypes>
#include <functional>
#include <iomanip>
#include "util/datetime.h"
#include "util/fixed_container.h"


namespace rk::data_type
{
    // types
    using OrderRef = std::uint32_t;
    // enums
    enum class Exchange
    {
        UNKNOWN,
        SSE,
        SZSE,
        SHFE,
        CFFEX,
        INE,
        DCE,
        CZCE,
        GFEX
    };
    enum class ProductClass
    {
        UNKNOWN,
        STOCK,
        ETF,
        FUTURE
    };
    enum class Direction
    {
        UNKNOWN,
        LONG,
        SHORT
    };
    enum class Offset
    {
        UNKNOWN,
        OPEN,
        CLOSE,
        CLOSE_TD,
        CLOSE_YD
    };
    enum class ErrorType
    {
        UNKNOWN,
        ORDER_INSERT_ERROR,
        ORDER_CANCEL_ERROR
    };
    enum class OrderStatus
    {
        UNKNOWN,            // 未知
        QUEUEING,           // 排队
        REJECTED,           // 拒单
        ALL_TRADED,         // 全成
        PARTIAL_TRADED,     // 部成
        ALL_CANCELED,       // 撤单
        PARTIAL_CANCELED    // 部成部撤
    };

    // structs
    struct Symbol
    {
        util::FixedString<16>                       symbol;
        util::FixedString<16>                       trade_symbol;
        Exchange                                    exchange = Exchange::UNKNOWN;
        ProductClass                                product_class = ProductClass::UNKNOWN;
        bool operator==(const Symbol& other) const {return symbol == other.symbol;}
        bool operator<(const Symbol& other) const {return symbol < other.symbol;}
    };
    struct TickData
    {
        Symbol                                      symbol;
        uint32_t                                    trading_day = 0;
        util::DateTime                              update_time;
        double                                      last_price = 0.;
        uint32_t                                    volume = 0;
        double                                      open_interest = 0.;
        double                                      average_price = 0.;
        std::array<std::pair<double, uint32_t>, 10> bid_order;
        std::array<std::pair<double, uint32_t>, 10> ask_order;
    };
    struct SymbolDetail
    {
        Symbol                                      symbol;
        ProductClass                                product_class = ProductClass::UNKNOWN;
        util::FixedString<16>                       underlying_asset;
        double                                      price_tick = 0.;
        int                                         multiplier = 0;
        uint32_t                                    min_buy_volume = 0;
        uint32_t                                    min_sell_volume = 0;
        uint32_t                                    max_buy_volume = 0;
        uint32_t                                    max_sell_volume = 0;
        double                                      upper_limit_price = 0.;
        double                                      lower_limit_price = 0.;
        uint32_t                                    open_date = 0;
        uint32_t                                    expire_date = 0;
        double                                      long_margin_ratio = 0.;
        double                                      short_margin_ratio = 0.;
        double                                      open_fee_rate_by_money = 0.;
        double                                      open_fee_rate_by_volume = 0.;
        double                                      close_fee_rate_by_money = 0.;
        double                                      close_fee_rate_by_volume = 0.;
        double                                      close_today_fee_rate_by_money = 0.;
        double                                      close_today_fee_rate_by_volume = 0.;
    };
    struct BarData
    {
        Symbol                                      symbol;
        uint32_t                                    trading_day = 0;
        util::DateTime                              update_time;
        double                                      open = 0.;
        double                                      high = 0.;
        double                                      low = 0.;
        double                                      close = 0.;
        uint32_t                                    volume = 0;
        double                                      amount = 0.;
    };
    struct PositionInfo
    {
        uint32_t                                    position = 0;
        uint32_t                                    frozen = 0;
        uint32_t                                    pending = 0;
        [[nodiscard]] bool empty() const {return !position && !frozen && !pending;}
        bool operator==(const PositionInfo& other) const
        {
            return (
                position == other.position &&
                frozen == other.frozen &&
                pending == other.pending
            );
        }
    };
    struct PositionData
    {
        Symbol                                      symbol;
        PositionInfo                                long_position;
        PositionInfo                                short_position;
        bool operator==(const PositionData& other) const
        {
            return (
                symbol == other.symbol &&
                long_position == other.long_position &&
                short_position == other.short_position
            );
        }
        [[nodiscard]] int32_t net_position() const
        {
            return static_cast<int32_t>(long_position.position - short_position.position);
        }
        [[nodiscard]] bool empty() const
        {
            return long_position.empty() && short_position.empty();
        }
    };
    struct OrderReq
    {
        data_type::Symbol                           symbol;
        double                                      limit_price = 0.;
        uint32_t                                    volume = 0;
        Direction                                   direction = Direction::UNKNOWN;
        Offset                                      offset = Offset::UNKNOWN;
        bool operator==(const OrderReq&) const = default;
    };
    struct OrderData
    {
        OrderRef                                    order_ref = 0;
        OrderReq                                    order_req;
        uint32_t                                    trading_day = 0;
        util::DateTime                              req_time;
        uint32_t                                    traded_volume = 0;
        uint32_t                                    remain_volume = 0;
        uint32_t                                    canceled_volume = 0;
        [[nodiscard]] bool is_rejected() const
        {
            return (
                order_req.volume != 0 &&
                order_req.volume != (traded_volume + remain_volume + canceled_volume)
            );
        }
        [[nodiscard]] bool is_finished() const
        {
            return (
                is_rejected() ||
                remain_volume == 0
            );
        }
    };
    struct TradeData
    {
        OrderRef                                    order_ref = 0;
        util::FixedString<64>                       trade_id;
        double                                      trade_price = 0.;
        uint32_t                                    trade_volume = 0;
        uint32_t                                    trading_day = 0;
        util::DateTime                              trade_time;
        double                                      fee = 0.;
    };
    struct AccountData
    {
        uint32_t                                    trading_day = 0;
        util::DateTime                              update_time;
        double                                      balance = 0;
        double                                      pre_balance = 0;
        double                                      deposit = 0;
        double                                      withdraw = 0;
        double                                      margin = 0;
        double                                      pre_margin = 0;
        double                                      frozen_margin = 0;
        double                                      commission = 0;
        double                                      available = 0;

    };
    struct CancelData
    {
        OrderRef                                    order_ref = 0;
        uint32_t                                    cancel_volume = 0;
        uint32_t                                    trading_day = 0;
        util::DateTime                              cancel_time;
    };
    struct OrderError
    {
        uint32_t                                    trading_day = 0;
        OrderRef                                    order_ref = 0;
        ErrorType                                   error_type = ErrorType::UNKNOWN;
        util::FixedString<128>                      error_msg;
    };
    struct AlgoReq
    {
        data_type::Symbol                           symbol;
        int32_t                                     net_position;
        util::FixedString<16>                       algo_name;
        util::FixedString<16>                       algo_param_json;
        util::DateTime                              start_time;
        util::DateTime                              end_time;

    };

};
namespace std
{
    template<>
    struct hash<rk::data_type::Symbol>
    {
        size_t operator()(const rk::data_type::Symbol& s) const noexcept
        {
            return std::hash<std::string_view>{}(s.symbol.view());
        };
    };
    template<>
    struct hash<rk::data_type::OrderReq>
    {
        size_t operator()(const rk::data_type::OrderReq& req) const noexcept
        {
            size_t h1 = std::hash<rk::data_type::Symbol>{}(req.symbol);
            size_t h2 = std::hash<double>{}(req.limit_price);
            size_t h3 = std::hash<uint32_t>{}(req.volume);
            size_t h4 = std::hash<int>{}(static_cast<int>(req.direction));
            size_t h5 = std::hash<int>{}(static_cast<int>(req.offset));

            // 组合哈希值
            size_t seed = 0;
            seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h5 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        };
    };
};