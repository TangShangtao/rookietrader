//
// Created by root on 2025/10/22.
//

#pragma once
#include <memory>
#include "data_type.h"
#include "engine_impl/engine_impl.h"

namespace rk::algo
{
    class Algo : public interface::Strategy
    {
    public:
        Algo(EngineImpl& engine, data_type::Symbol symbol): _engine(engine), _symbol(std::move(symbol)) {}
        ~Algo() override = default;
        std::unordered_set<data_type::Symbol> on_init(uint32_t trading_day) final
        {
            // TODO 共享数据
            _trading_day = trading_day;
            _last_tick = _engine.query_last_tick(_symbol);
            _symbol_detail = _engine.query_symbol_detail(_symbol);
            _position_data = _engine.query_position_data(_symbol);

            std::unordered_set<data_type::Symbol> symbols_to_sub{};
            on_algo_init(symbols_to_sub);
            symbols_to_sub.emplace(_symbol);
            return symbols_to_sub;
        }
        void set_strategy_id(uint32_t strategy_id) {_strategy_id = strategy_id;}
        virtual void on_algo_init(std::unordered_set<data_type::Symbol>& symbols_to_sub) {};
        void on_tick(const data_type::TickData& data) override {};
        void on_bar(const data_type::BarData& data) override {};
        void on_trade(const data_type::TradeData& data) override {};
        void on_cancel(const data_type::CancelData& data) override {};
        void on_error(const data_type::OrderError& data) override {};
        void on_algo_req(const data_type::AlgoReq& data) override {};
    protected:
        uint32_t _trading_day = 0;
        uint32_t _strategy_id = 0;
        EngineImpl& _engine;
        data_type::Symbol _symbol;
        std::shared_ptr<const data_type::TickData> _last_tick;
        std::shared_ptr<const data_type::SymbolDetail> _symbol_detail;
        std::shared_ptr<const data_type::PositionData> _position_data;
    };

    std::shared_ptr<Algo> create_algo(
        EngineImpl& engine,
        const data_type::Symbol& symbol,
        const std::string& algo_name,
        const std::string& algo_param_json
    );
};