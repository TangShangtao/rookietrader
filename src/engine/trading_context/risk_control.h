#pragma once
#include <memory>
#include "data_type.h"
#include "config_type.h"
#include "db.h"
namespace rk
{
    struct TradeInfo;
    struct MarketInfo;
    struct RiskIndicators;
    class RiskControl
    {
    public:
        RiskControl(
            const bool& is_trading,
            const config_type::AccountConfig& account_config,
            const config_type::RiskControlConfig& risk_control_config,
            db::Executor& db_writer
        );
        std::shared_ptr<const RiskIndicators> set_trade_info(std::shared_ptr<const TradeInfo> trade_info);
        std::shared_ptr<const RiskIndicators> set_market_info(std::shared_ptr<const MarketInfo> market_info);

        bool check_order_insert(const data_type::OrderReq& req);
        bool check_order_cancel(data_type::OrderRef order_ref);

        bool check_handle_tick(const data_type::TickData& data);
        bool check_handle_bar(const data_type::BarData& data);
        bool check_handle_trade(const data_type::TradeData& data);
        bool check_handle_cancel(const data_type::CancelData& data);
        bool check_handle_error(const data_type::OrderError& data);

    private:
        const bool& _is_trading;
        std::shared_ptr<const TradeInfo> _trade_info = std::make_shared<const TradeInfo>();
        std::shared_ptr<const MarketInfo> _market_info = std::make_shared<const MarketInfo>();
        std::shared_ptr<RiskIndicators> _risk_indicators;
        const config_type::AccountConfig& _account_config;
        std::unique_ptr<RiskIndicators> _thresholds;
        db::Executor& _db_writer;
    };

};