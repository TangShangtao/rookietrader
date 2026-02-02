#pragma once
#include "data_type.h"
#include "interface.h"
#include "engine_impl.h"
#include <replxx.hxx>
#include <VariadicTable.h>
#include <magic_enum/magic_enum.hpp>
#include <ranges>

namespace rk::rk_terminal
{
    class RKTerminal : public interface::Strategy
    {
    public:
        RKTerminal(
            EngineImpl& engine,
            replxx::Replxx& rx,
            std::function<void(const std::string&)> publish_cb
        )
            : _engine(engine), _rx(rx), _publish_cb(std::move(publish_cb)) {}
        std::unordered_set<data_type::Symbol> on_init(uint32_t trading_day) override
        {
            const auto& symbol_detail = _engine._market_info->_symbol_details;
            auto keys = symbol_detail | std::views::transform([](const auto& kv) {return kv.first;});
            return {keys.begin(), keys.end()};
        }
        void on_tick(const data_type::TickData& data) override {};
        void on_bar(const data_type::BarData& data) override {};
        void on_trade(const data_type::TradeData& data) override
        {
            VariadicTable<std::string, data_type::OrderRef, std::string, double, uint32_t, std::string, double> vt{
                {"callback", "order_ref", "trade_id", "trade_price", "trade_volume", "trade_time", "fee"}
            };
            vt.addRow(
                __FUNCTION__,
                data.order_ref, data.trade_id.to_string(), data.trade_price, data.trade_volume,
                data.trade_time.strftime(),
                data.fee
            );
            std::ostringstream oss;
            vt.print(oss);
            _rx.print("%s\n%s\n", util::DateTime::now().strftime().c_str(), oss.str().c_str());
        }
        void on_cancel(const data_type::CancelData& data) override
        {
            VariadicTable<std::string, data_type::OrderRef, uint32_t, std::string> vt{
                {"callback", "order_ref", "cancel_volume", "cancel_time"}
            };
            vt.addRow(__FUNCTION__, data.order_ref, data.cancel_volume, data.cancel_time.strftime());
            std::ostringstream oss;
            vt.print(oss);
            _rx.print("%s\n%s\n", util::DateTime::now().strftime().c_str(), oss.str().c_str());
        }
        void on_error(const data_type::OrderError& data) override
        {
            VariadicTable<std::string, data_type::OrderRef, std::string, std::string> vt{
                {"callback", "order_ref", "error_type", "error_msg"}
            };
            vt.addRow(__FUNCTION__, data.order_ref, magic_enum::enum_name(data.error_type).data(), data.error_msg.to_string());
            std::ostringstream oss;
            vt.print(oss);
            _rx.print("%s\n%s\n", util::DateTime::now().strftime().c_str(), oss.str().c_str());
        }
    private:
        EngineImpl& _engine;
        replxx::Replxx& _rx;
        std::function<void(const std::string&)> _publish_cb;
    };
};

