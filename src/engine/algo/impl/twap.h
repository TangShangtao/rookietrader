//
// Created by root on 2025/10/24.
//

#pragma once
#include "algo/algo.h"
#include "util/datetime.h"
namespace rk::algo
{

    struct TwapAlgoParam
    {
        int retry_interval_secs = 30;
        explicit TwapAlgoParam(const std::string& algo_param_json);
    };
    class Algo;
    class Twap final : public Algo
    {
        enum class AlgoStatus
        {
            UNKNOWN,
            STOPPED,
            ERROR,
            SENDING,
            CANCELING,
            EXECUTING,
            IDLE
        };
    public:
        Twap(EngineImpl& engine, const data_type::Symbol& symbol, const std::string& algo_param_json);
        Twap(EngineImpl& engine, const data_type::Symbol& symbol, TwapAlgoParam algo_param);
        ~Twap() override = default;
        Twap() = delete;
        Twap(Twap&&) = delete;
        Twap(const Twap&) = delete;
        void on_tick(const data_type::TickData& data) override;
        void on_trade(const data_type::TradeData& data) override;
        void on_cancel(const data_type::CancelData& data) override;
        void on_error(const data_type::OrderError& data) override;
        void on_algo_req(const data_type::AlgoReq& data) override;
    private:
        void retry_order(const util::DateTime& datetime);
        data_type::OrderReq algin_position(const util::DateTime& datetime);
    private:
        AlgoStatus _algo_status = AlgoStatus::UNKNOWN;
        TwapAlgoParam _algo_param;
        util::DateTime _last_retry_dt = util::DateTime::now();
        data_type::AlgoReq _algo_req;
        std::optional<data_type::OrderRef> _executing_order_ref = std::nullopt;
    };
};