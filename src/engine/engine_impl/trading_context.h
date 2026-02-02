#pragma once
#include <memory>
#include <unordered_set>
#include "adapter/adapter.h"
#include "data_type.h"


namespace rk
{
    struct MarketHandler
    {
        std::function<void(const data_type::TickData& data)> on_tick = nullptr;
        std::function<void(const data_type::BarData& data)> on_bar = nullptr;

    };
    struct TradeHandler
    {
        std::function<void(const data_type::TradeData& data)> on_trade = nullptr;
        std::function<void(const data_type::CancelData& data)> on_cancel = nullptr;
        std::function<void(const data_type::OrderError& data)> on_error = nullptr;
    };
    class TradingContext
    {
    public:
        TradingContext();
        ~TradingContext() = default;
        TradingContext(const TradingContext&) = delete;
        TradingContext& operator=(const TradingContext&) = delete;
        TradingContext(TradingContext&&) = delete;
        TradingContext& operator=(TradingContext&&) = delete;
        void subscribe(const MarketHandler& handler, const data_type::Symbol& symbol);
        void order_insert(TradeHandler handler, data_type::OrderRef order_ref);

        void handle_tick(const data_type::TickData& data);
        void handle_bar(const data_type::BarData& data);
        void handle_trade(const data_type::TradeData& data);
        void handle_cancel(const data_type::CancelData& data);
        void handle_error(const data_type::OrderError& data);

        // handlers
        std::unordered_multimap<data_type::Symbol, MarketHandler> _market_handlers;
        std::vector<TradeHandler> _trade_handlers;

    };


};