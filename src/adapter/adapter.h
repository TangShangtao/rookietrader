#pragma once
#include "data_type.h"
#include "config_type.h"
#include <unordered_set>
#include <condition_variable>
namespace rk::adapter
{
    enum RPCResult
    {
        UNKNOWN,
        SUCCESS,
        FAILED
    };
    class MDAdapter
    {
    public:
        struct PushDataCallbacks
        {
            std::function<void(data_type::TickData&&)> push_tick;
            std::function<void()> push_md_disconnected;
        };
        MDAdapter(PushDataCallbacks push_data_callbacks, config_type::MDAdapterConfig config)
        : _push_data_callbacks(std::move(push_data_callbacks)), _config(std::move(config))
        {

        }
        virtual ~MDAdapter() = default;
        virtual bool login() = 0;
        virtual void logout() = 0;
        virtual bool subscribe(std::unordered_set<data_type::Symbol>) = 0;
        virtual bool unsubscribe(std::unordered_set<data_type::Symbol>) = 0;
        virtual std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>>> query_symbol_detail() = 0;

    protected:
        PushDataCallbacks                       _push_data_callbacks;
        config_type::MDAdapterConfig            _config;
    };

    class TDAdapter
    {
    public:
        struct PushDataCallbacks
        {
            std::function<void(data_type::TradeData&&)> push_trade;
            std::function<void(data_type::CancelData&&)> push_cancel;
            std::function<void(data_type::OrderError&&)> push_order_error;
            std::function<void()> push_td_disconnected;
        };
        TDAdapter(PushDataCallbacks push_data_callbacks, config_type::TDAdapterConfig config)
        : _push_data_callbacks(std::move(push_data_callbacks)), _config(std::move(config))
        {

        }
        TDAdapter() = delete;
        TDAdapter(const TDAdapter&) = delete;
        TDAdapter(TDAdapter&&) = delete;

        virtual ~TDAdapter() = default;
        virtual bool login() = 0;
        virtual void logout() = 0;
        virtual uint32_t query_trading_day() = 0;
        virtual std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::PositionData>>> query_position_data() = 0;
        virtual std::optional<std::vector<data_type::OrderData>> query_order_data() = 0;
        virtual std::optional<std::vector<std::vector<data_type::TradeData>>> query_trade_data() = 0;
        virtual std::optional<data_type::AccountData> query_account_data() = 0;
        virtual void order_insert(data_type::OrderRef order_ref, const data_type::OrderReq& order_req) = 0;
        virtual void order_cancel(data_type::OrderRef order_ref) = 0;

    protected:
        PushDataCallbacks                       _push_data_callbacks;
        config_type::TDAdapterConfig            _config;
    };

    std::unique_ptr<MDAdapter> create_md_adapter(
        MDAdapter::PushDataCallbacks push_data_callbacks,
        config_type::MDAdapterConfig config
    );
    std::unique_ptr<TDAdapter> create_td_adapter(
        TDAdapter::PushDataCallbacks push_data_callbacks,
        config_type::TDAdapterConfig config
    );
}