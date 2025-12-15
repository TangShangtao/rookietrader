//
// Created by root on 25-9-15.
//

#pragma once
#include <unordered_set>
#include <chrono>
#include "data_type.h"

namespace rk::interface
{
    class Strategy
    {
    public:
        virtual ~Strategy() = default;
        virtual std::unordered_set<data_type::Symbol> on_init(uint32_t trading_day) = 0;
        virtual void on_tick(const data_type::TickData& data) {};
        virtual void on_bar(const data_type::BarData& data) {};
        virtual void on_trade(const data_type::TradeData& data) {};
        virtual void on_cancel(const data_type::CancelData& data) {};
        virtual void on_error(const data_type::OrderError& data) {};
        virtual void on_algo_req(const data_type::AlgoReq& data) {};
    };

};