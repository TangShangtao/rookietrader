//
// Created by root on 2025/9/30.
//

#pragma once
#include "data_type.h"
#include <nlohmann/json.hpp>
namespace rk::data_type
{
    NLOHMANN_JSON_SERIALIZE_ENUM(Exchange, {
        {Exchange::UNKNOWN, "UNKNOWN"},
        {Exchange::SSE,     "SSE"},
        {Exchange::SZSE,    "SZSE"},
        {Exchange::SHFE,    "SHFE"},
        {Exchange::CFFEX,   "CFFEX"},
        {Exchange::INE,     "INE"},
        {Exchange::DCE,     "DCE"},
        {Exchange::CZCE,    "CZCE"},
        {Exchange::GFEX,    "GFEX"},
    })

    NLOHMANN_JSON_SERIALIZE_ENUM(ProductClass, {
        {ProductClass::UNKNOWN, "UNKNOWN"},
        {ProductClass::STOCK,   "STOCK"},
        {ProductClass::ETF,     "ETF"},
        {ProductClass::FUTURE,  "FUTURE"},
    })

    NLOHMANN_JSON_SERIALIZE_ENUM(Direction, {
        {Direction::UNKNOWN, "UNKNOWN"},
        {Direction::LONG,    "LONG"},
        {Direction::SHORT,   "SHORT"},
    })

    NLOHMANN_JSON_SERIALIZE_ENUM(Offset, {
        {Offset::UNKNOWN,        "UNKNOWN"},
        {Offset::OPEN,           "OPEN"},
        {Offset::CLOSE,          "CLOSE"},
        {Offset::CLOSE_TD,       "CLOSE_TD"},
        {Offset::CLOSE_YD,       "CLOSE_YD"},
    })

    NLOHMANN_JSON_SERIALIZE_ENUM(ErrorType, {
        {ErrorType::UNKNOWN,           "UNKNOWN"},
        {ErrorType::ORDER_INSERT_ERROR,"ORDER_INSERT_ERROR"},
        {ErrorType::ORDER_CANCEL_ERROR,"ORDER_CANCEL_ERROR"},
    })

    // NLOHMANN_JSON_SERIALIZE_ENUM(OrderStatus, {
    //     {OrderStatus::UNKNOWN,          "UNKNOWN"},
    //     {OrderStatus::QUEUEING,         "QUEUEING"},
    //     {OrderStatus::REJECTED,         "REJECTED"},
    //     {OrderStatus::ALL_TRADED,       "ALL_TRADED"},
    //     {OrderStatus::PARTIAL_TRADED,   "PARTIAL_TRADED"},
    //     {OrderStatus::ALL_CANCELED,     "ALL_CANCELED"},
    //     {OrderStatus::PARTIAL_CANCELED, "PARTIAL_CANCELED"},
    // })
    template <typename T>
    auto to_json(const T& data)
    {
        nlohmann::ordered_json j;
        to_json(j, data);
        return j;
    }
    inline void to_json(nlohmann::ordered_json& j, const Symbol& s) {
        j = nlohmann::ordered_json{{"symbol", s.symbol.to_string()}, {"trade_symbol", s.trade_symbol.to_string()}, {"exchange", s.exchange}};
    }
    inline void to_json(nlohmann::ordered_json& j, const TickData& t) {
        j = nlohmann::ordered_json{
            {"symbol", t.symbol},
            {"trading_day", t.trading_day},
            {"update_time", t.update_time.strftime()},
            {"last_price", t.last_price},
            {"volume", t.volume},
            {"open_interest", t.open_interest},
            {"average_price", t.average_price},
            {"bid_order", t.bid_order},
            {"ask_order", t.ask_order}
        };
    }
    inline void to_json(nlohmann::ordered_json& j, const SymbolDetail& s) {
        j = nlohmann::ordered_json{
            {"symbol", s.symbol},
            {"product_class", s.product_class},
            {"underlying_asset", s.underlying_asset.to_string()},
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
            {"direction", o.direction},
            {"offset", o.offset},
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
            {"error_type", e.error_type},
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