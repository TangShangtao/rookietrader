#include "trading_context.h"
#include <utility>
namespace rk
{
    TradingContext::TradingContext()
        :   _trade_handlers(1000)
    {

    }
    void TradingContext::subscribe(const MarketHandler& handler, const data_type::Symbol& symbol)
    {
        _market_handlers.emplace(symbol, handler);
    }
    void TradingContext::order_insert(TradeHandler handler, data_type::OrderRef order_ref)
    {
        _trade_handlers[order_ref] = std::move(handler);
    }
    void TradingContext::handle_tick(const data_type::TickData& data)
    {
        auto range = _market_handlers.equal_range(data.symbol);
        for (auto it = range.first; it != range.second; ++it)
        {
            if ((it->second).on_tick != nullptr)
            {
                (it->second).on_tick(data);
            }
        }
    }
    void TradingContext::handle_bar(const data_type::BarData& data)
    {

    }
    void TradingContext::handle_trade(const data_type::TradeData& data)
    {
        if (_trade_handlers[data.order_ref].on_trade != nullptr)
        {
            _trade_handlers[data.order_ref].on_trade(data);
        }
    }
    void TradingContext::handle_cancel(const data_type::CancelData& data)
    {
        if (_trade_handlers[data.order_ref].on_cancel != nullptr)
        {
            _trade_handlers[data.order_ref].on_cancel(data);
        }
    }
    void TradingContext::handle_error(const data_type::OrderError& data)
    {
        if (_trade_handlers[data.order_ref].on_error != nullptr)
        {
            _trade_handlers[data.order_ref].on_error(data);
        }
    }
};