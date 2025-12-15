#pragma once
#include "data_type.h"
#include "config_type.h"
#include "event.h"
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <utility>
#include <magic_enum/magic_enum.hpp>

namespace rk::gateway
{
    enum class GatewayStatus
    {
        UNKNOWN,
        TRADE_LOGIN,
        MARKET_LOGIN,
        SYMBOL_DETAIL_QUERIED,
        POSITION_DATA_QUERIED,
        ORDER_DATA_QUERIED,
        TRADE_DATA_QUERIED,
        ACCOUNT_DATA_QUERIED,
        SUBSCRIBED,
        ERROR
    };
    class Gateway
    {
    public:
        Gateway(
            std::shared_ptr<event::EventLoop> event_loop,
            config_type::GatewayConfig config
        ) :
        _event_loop(std::move(event_loop)),
        _config(std::move(config))
        {
            for (const auto& product_class_str : _config.product_class)
            {
                _product_class.emplace_back(magic_enum::enum_cast<data_type::ProductClass>(product_class_str).value());
            }
        }
        virtual ~Gateway() = default;
        virtual bool login_trade() = 0;
        virtual bool login_market() = 0;
        virtual void logout_trade() = 0;
        virtual void logout_market() = 0;
        virtual bool subscribe(std::unordered_set<data_type::Symbol>) = 0;
        virtual uint32_t query_trading_day() = 0;
        virtual std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>>> query_symbol_details() = 0;
        virtual std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::PositionData>>> query_position_data() = 0;
        virtual std::optional<std::vector<data_type::OrderData>> query_order_data() = 0;
        virtual std::optional<std::vector<std::vector<data_type::TradeData>>> query_trade_data() = 0;
        virtual std::optional<data_type::AccountData> query_account_data() = 0;
        virtual void order_insert(data_type::OrderRef order_ref, const data_type::OrderReq& order_req) = 0;
        virtual void order_cancel(data_type::OrderRef order_ref) = 0;

    protected:
        std::shared_ptr<event::EventLoop>       _event_loop;
        GatewayStatus                           _gateway_status = GatewayStatus::UNKNOWN;
        config_type::GatewayConfig              _config;
        std::vector<data_type::ProductClass>    _product_class;
    };

    std::unique_ptr<Gateway> create_gateway(
        std::shared_ptr<event::EventLoop> event_loop,
        config_type::GatewayConfig config
    );
};