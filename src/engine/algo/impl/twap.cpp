//
// Created by root on 2025/10/24.
//
#include "twap.h"
#include <cmath>
#include <nlohmann/json.hpp>
#include "util/logger.h"
namespace rk::algo
{
    TwapAlgoParam::TwapAlgoParam(const std::string& algo_param_json)
    {

        const auto json = nlohmann::json::parse(algo_param_json, nullptr, false);

        if (
            !json.is_discarded() &&
            json.find("retry_interval_secs") != json.end()
        )
        {
            retry_interval_secs = json["retry_interval_secs"];
        }
    }
    Twap::Twap(EngineImpl& engine, const data_type::Symbol& symbol, TwapAlgoParam algo_param)
        :   Algo(engine, symbol), _algo_param(algo_param)
    {

    }
    Twap::Twap(EngineImpl& engine, const data_type::Symbol& symbol, const std::string& algo_param_json)
        : Twap(engine, symbol, TwapAlgoParam(algo_param_json))
    {

    }
    void Twap::on_algo_req(const data_type::AlgoReq& data)
    {
        if (
            (_algo_status == AlgoStatus::ERROR) ||
            (_algo_status == AlgoStatus::STOPPED)
        )
            return;
        _algo_req = data;
        RK_LOG_INFO("{}", data_type::to_json(data).dump(4).c_str());
    }
    void Twap::on_tick(const data_type::TickData& data)
    {
        if (data.update_time < _algo_req.start_time)
        {
            RK_LOG_INFO("update_time {}, start_time {}", data.update_time.strftime().c_str(), _algo_req.start_time.strftime().c_str());
            return;
        }
        else if (data.update_time < _algo_req.end_time)
        {
            retry_order(data.update_time);
        }
        else
        {
            _algo_status = AlgoStatus::STOPPED;
        }
    }
    void Twap::on_trade(const data_type::TradeData& data)
    {

    }
    void Twap::on_cancel(const data_type::CancelData& data)
    {

    }
    void Twap::on_error(const data_type::OrderError& data)
    {
    }
    void Twap::retry_order(const util::DateTime& datetime)
    {

    }
    // TODO只处理股票ETF等T+1
    data_type::OrderReq Twap::algin_position(const util::DateTime& datetime) {
        auto split_count = (_algo_req.end_time - datetime).seconds() / _algo_param.retry_interval_secs;
        split_count = split_count == 0 ? 1 : split_count;
        auto delta_position = (
            _algo_req.net_position -
            _position_data->long_position.position
        ) / split_count;
        auto direction = delta_position >= 0 ? data_type::Direction::LONG : data_type::Direction::SHORT;
        auto offset = delta_position >= 0 ? data_type::Offset::OPEN : data_type::Offset::CLOSE;
        delta_position = delta_position >= 0 ? delta_position : -delta_position;
        long min_order_volume = 100;
        split_count = delta_position < min_order_volume ? 1 : split_count;
        long suborder_volume = delta_position / split_count;
        while (split_count)
        {
            suborder_volume = std::round(suborder_volume / min_order_volume) * min_order_volume;
            if (suborder_volume) break;
            split_count -= 1;
            suborder_volume = delta_position / split_count;
        }
        return {
            _symbol,
            _last_tick->last_price,
            static_cast<uint32_t>(suborder_volume),
            direction,
            offset
        };

    }
};