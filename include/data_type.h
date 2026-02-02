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
#include <span>
#include <nlohmann/json.hpp>
#include <magic_enum/magic_enum.hpp>

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
        BSE,
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
    enum class SymbolTradingStatus
    {
        UNKNOWN,
        TEMPORARY_SUSPENSION,   // 临时停牌
        SUSPENSION,             // 当日停牌
        TRADEABLE               // 可以交易
    };
    enum class DataType
    {

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
        double                                      open_price = 0.;
        double                                      highest_price = 0.;
        double                                      lowest_price = 0.;
        double                                      upper_limit_price = std::numeric_limits<double>::max();
        double                                      lower_limit_price = 0.;
        int64_t                                     volume = 0;
        double                                      open_interest = 0.;
        double                                      average_price = 0.;
        std::array<double, 10>                      bid_price{};
        std::array<double, 10>                      ask_price{};
        std::array<int64_t, 10>                     bid_volume{};
        std::array<int64_t, 10>                     ask_volume{};
        double                                      iopv = 0.;
    };

    struct SymbolDetail
    {
        Symbol                                      symbol;
        ProductClass                                product_class = ProductClass::UNKNOWN;
        util::FixedString<16>                       underlying_asset;
        util::FixedString<32>                       symbol_name;
        double                                      price_tick = 0.;
        int                                         multiplier = 0;
        uint32_t                                    min_buy_volume = 0;
        uint32_t                                    min_sell_volume = 0;
        uint32_t                                    max_buy_volume = 0;
        uint32_t                                    max_sell_volume = 0;
        double                                      upper_limit_price = std::numeric_limits<double>::max();
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
        double                                      market_value = 0.;
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

namespace rk::data_type
{
    template <typename T>
    auto to_json(const T& data)
    {
        nlohmann::ordered_json j;
        to_json(j, data);
        return j;
    }
    inline void to_json(nlohmann::ordered_json& j, const Symbol& s) {
        j = nlohmann::ordered_json{
            {"symbol", s.symbol.to_string()},
            {"trade_symbol", s.trade_symbol.to_string()},
            {"exchange", magic_enum::enum_name(s.exchange)},
            {"product_class", magic_enum::enum_name(s.product_class)}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const TickData& t) {
        j = nlohmann::ordered_json{
            {"symbol", t.symbol},
            {"trading_day", t.trading_day},
            {"update_time", t.update_time.strftime()},
            {"last_price", t.last_price},
            {"open_price", t.open_price},
            {"highest_price", t.highest_price},
            {"lowest_price", t.lowest_price},
            {"upper_limit_price", t.upper_limit_price},
            {"lower_limit_price", t.lower_limit_price},
            {"volume", t.volume},
            {"open_interest", t.open_interest},
            {"average_price", t.average_price},
            {"bid_price", t.bid_price},
            {"ask_price", t.ask_price},
            {"bid_volume", t.bid_volume},
            {"ask_volume", t.ask_volume},
            {"iopv", t.iopv}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const SymbolDetail& s) {
        j = nlohmann::ordered_json{
            {"symbol", s.symbol},
            {"product_class", magic_enum::enum_name(s.product_class)},
            {"underlying_asset", s.underlying_asset.to_string()},
            {"symbol_name", s.symbol_name.to_string()},
            {"price_tick", s.price_tick},
            {"multiplier", s.multiplier},
            {"min_buy_volume", s.min_buy_volume},
            {"min_sell_volume", s.min_sell_volume},
            {"max_buy_volume", s.max_buy_volume},
            {"max_sell_volume", s.max_sell_volume},
            {"upper_limit_price", s.upper_limit_price},
            {"lower_limit_price", s.lower_limit_price},
            {"open_date", s.open_date},
            {"expire_date", s.expire_date},
            {"long_margin_ratio", s.long_margin_ratio},
            {"short_margin_ratio", s.short_margin_ratio},
            {"open_fee_rate_by_money", s.open_fee_rate_by_money},
            {"open_fee_rate_by_volume", s.open_fee_rate_by_volume},
            {"close_fee_rate_by_money", s.close_fee_rate_by_money},
            {"close_fee_rate_by_volume", s.close_fee_rate_by_volume},
            {"close_today_fee_rate_by_money", s.close_today_fee_rate_by_money},
            {"close_today_fee_rate_by_volume", s.close_today_fee_rate_by_volume}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const BarData& b) {
        j = nlohmann::ordered_json{
            {"symbol", b.symbol},
            {"trading_day", b.trading_day},
            {"update_time", b.update_time.strftime()},
            {"open", b.open},
            {"high", b.high},
            {"low", b.low},
            {"close", b.close},
            {"volume", b.volume},
            {"amount", b.amount}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const PositionInfo& p) {
        j = nlohmann::ordered_json{
            {"position", p.position},
            {"frozen", p.frozen},
            {"pending", p.pending}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const PositionData& p) {
        j = nlohmann::ordered_json{
            {"symbol", p.symbol},
            {"long_position", p.long_position},
            {"short_position", p.short_position}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const OrderReq& o) {
        j = nlohmann::ordered_json{
            {"symbol", o.symbol},
            {"limit_price", o.limit_price},
            {"volume", o.volume},
            {"direction", magic_enum::enum_name(o.direction)},
            {"offset", magic_enum::enum_name(o.offset)},
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const OrderData& o) {
        j = nlohmann::ordered_json{
            {"order_ref", o.order_ref},
            {"order_req", o.order_req},
            {"trading_day", o.trading_day},
            {"req_time", o.req_time.strftime()},
            // {"status", o.status},
            {"traded_volume", o.traded_volume},
            {"remain_volume", o.remain_volume},
            {"canceled_volume", o.canceled_volume}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const TradeData& t) {
        j = nlohmann::ordered_json{
            {"order_ref", t.order_ref},
            {"trade_id", t.trade_id.to_string()},
            {"trade_price", t.trade_price},
            {"trade_volume", t.trade_volume},
            {"trading_day", t.trading_day},
            {"trade_time", t.trade_time.strftime()},
            {"fee", t.fee}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const AccountData& t) {
        j = nlohmann::ordered_json{
            {"trading_day", t.trading_day},
            {"update_time", t.update_time.strftime()},
            {"balance", t.balance},
            {"market_value", t.market_value},
            {"pre_balance", t.pre_balance},
            {"deposit", t.deposit},
            {"withdraw", t.withdraw},
            {"margin", t.margin},
            {"pre_margin", t.pre_margin},
            {"frozen_margin", t.frozen_margin},
            {"commission", t.commission},
            {"available", t.available},
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const CancelData& c) {
        j = nlohmann::ordered_json{
            {"order_ref", c.order_ref},
            {"cancel_volume", c.cancel_volume},
            {"trading_day", c.trading_day},
            {"cancel_time", c.cancel_time.strftime()}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const OrderError& e) {
        j = nlohmann::ordered_json{
            {"trading_day", e.trading_day},
            {"order_ref", e.order_ref},
            {"error_type", magic_enum::enum_name(e.error_type)},
            {"error_msg", e.error_msg.to_string()}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const AlgoReq& e) {
        j = nlohmann::ordered_json{
            {"symbol", e.symbol},
            {"net_position", e.net_position},
            {"algo_name", e.algo_name.to_string()},
            {"algo_param_json", e.algo_param_json.to_string()},
            {"start_time", e.start_time.strftime()},
            {"end_time", e.end_time.strftime()},
        };
    }

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
namespace rk::ipc
{
    enum class IPCDataType : uint8_t
    {
        UNKNOWN = 0,
        REQ,
        RSP,
        PUB
    };
    struct IPCData
    {
        IPCDataType type = IPCDataType::UNKNOWN;
        util::DateTime timestamp = util::DateTime::now();
    };
    enum class RPCType : uint8_t
    {
        UNKNOWN = 0,
        SUBSCRIBE,
        UNSUBSCRIBE,
        QUERY_SYMBOL_DETAIL,
    };
    struct ReqData
    {
        RPCType type = RPCType::UNKNOWN;
        size_t list_len = 0;
        util::DateTime timestamp = util::DateTime::now();
    };
    struct SubscribeReq
    {
        std::span<const data_type::Symbol> symbol_list;
    };
    struct UnsubscribeReq
    {
        std::span<const data_type::Symbol> symbol_list;
    };
    struct QuerySymbolDetailReq
    {
        std::span<const data_type::Symbol> symbol_list;
    };
    struct RspData
    {
        bool res = false;
        RPCType type = RPCType::UNKNOWN;
        size_t list_len = 0;
        util::DateTime timestamp = util::DateTime::now();
    };
    struct SubscribeRsp
    {

    };
    struct UnsubscribeRsp
    {

    };
    struct QuerySymbolDetailRsp
    {
        std::span<const data_type::SymbolDetail> symbol_detail_list;
    };
    enum class PubDataType: uint8_t
    {
        UNKNOWN = 0,
        TICK_DATA,
    };
    struct PubData
    {
        PubDataType type = PubDataType::UNKNOWN;
        util::DateTime timestamp = util::DateTime::now();
    };
};

namespace rk::ipc
{

    template <typename T>
    auto to_json(const T& data)
    {
        nlohmann::ordered_json j;
        to_json(j, data);
        return j;
    }
    inline void to_json(nlohmann::ordered_json& j, const IPCData& s) {
        j = nlohmann::ordered_json{{"type", magic_enum::enum_name(s.type)}, {"timestamp", s.timestamp.strftime()}};
    }
    inline void to_json(nlohmann::ordered_json& j, const ReqData& s) {
        j = nlohmann::ordered_json{{"type", magic_enum::enum_name(s.type)}, {"list_len", s.list_len}, {"timestamp", s.timestamp.strftime()}};
    }
    inline void to_json(nlohmann::ordered_json& j, const RspData& s) {
        j = nlohmann::ordered_json{
            {"res", s.res}, {"type", magic_enum::enum_name(s.type)}, {"list_len", s.list_len}, {"timestamp", s.timestamp.strftime()}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const PubData& s) {
        j = nlohmann::ordered_json{
            {"type", magic_enum::enum_name(s.type)}, {"timestamp", s.timestamp.strftime()}
        };
    }
};