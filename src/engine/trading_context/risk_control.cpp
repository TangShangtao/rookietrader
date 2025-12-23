#include "risk_control.h"
#include "engine_impl/engine_impl.h"
#include "serialization.h"
#include <NanoLogCpp17.h>
#include <magic_enum/magic_enum.hpp>
#include <cmath>
namespace rk
{
    bool a_is_integer_multiple_of_b(double a, double b)
    {
        double q = a / b;
        double n = std::round(q);
        return std::fabs(q - n) <= std::max(1e-9 * std::fabs(q), 1e-12);
    }
    RiskControl::RiskControl(
        const bool& is_trading,
        const config_type::AccountConfig& account_config,
        const config_type::RiskControlConfig& risk_control_config,
        db::Executor& db_writer
    )
    : _is_trading(is_trading), _account_config(account_config), _db_writer(db_writer)
    {
        _thresholds = std::make_unique<RiskIndicators>(
            risk_control_config.daily_order_num,
            risk_control_config.daily_cancel_num,
            risk_control_config.daily_repeat_order_num
        );
        _risk_indicators = std::make_shared<RiskIndicators>();
    }
    std::shared_ptr<const RiskIndicators> RiskControl::set_trade_info(std::shared_ptr<const TradeInfo> trade_info)
    {
        _trade_info = std::move(trade_info);
        _risk_indicators->daily_order_num = 0;
        _risk_indicators->daily_cancel_num = 0;
        _risk_indicators->daily_repeat_order_num = 0;
        std::unordered_map<data_type::OrderReq, int> repeat_order_cnt;
        for (const auto& order : _trade_info->_order_data)
        {
            ++repeat_order_cnt[order.order_req];
            ++(_risk_indicators->daily_order_num);
            if (order.canceled_volume != 0) ++(_risk_indicators->daily_cancel_num);
        }
        _risk_indicators->daily_repeat_order_num = static_cast<int>(std::count_if(
            repeat_order_cnt.begin(),
            repeat_order_cnt.end(),
            [](const auto& pair){return pair.second > 1;}
        ));
        NANO_LOG(NOTICE, "risk indicators inited! daily_order_num: %i, daily_cancel_num: %i, daily_repeat_order_num: %i", _risk_indicators->daily_order_num, _risk_indicators->daily_cancel_num, _risk_indicators->daily_repeat_order_num);
        return _risk_indicators;
    }
    std::shared_ptr<const RiskIndicators> RiskControl::set_market_info(std::shared_ptr<const MarketInfo> market_info)
    {
        _market_info = std::move(market_info);
        return _risk_indicators;
    }

    bool RiskControl::check_order_insert(const data_type::OrderReq& req)
    {
        std::string log;
        bool pass = true;
        if (!_is_trading)
        {
            log = "trading stopped! order insert failed";
            pass = false;
        }
        // 报单笔数阈值
        else if (_risk_indicators->daily_order_num + 1 > _thresholds->daily_order_num)
        {
            log = std::format("daily order num({}) exceed max num({})! %s", _risk_indicators->daily_order_num + 1, _thresholds->daily_order_num);
            pass = false;
        }
        // 交易指令检查
        else if (!_market_info->_symbol_details.contains(req.symbol))
        {
            log = "symbol not found!";
            pass = false;
        }
        // 最小价位变动检查
        else if (!a_is_integer_multiple_of_b(req.limit_price, _market_info->_symbol_details.find(req.symbol)->second->price_tick))
        {
            log = std::format( "price tick {}, limit price illegal", _market_info->_symbol_details.find(req.symbol)->second->price_tick);
            pass = false;
        }
        // 最大报单手数检查
        else if (
            (req.direction == data_type::Direction::LONG && req.volume > _market_info->_symbol_details.find(req.symbol)->second->max_buy_volume) ||
            (req.direction == data_type::Direction::SHORT && req.volume > _market_info->_symbol_details.find(req.symbol)->second->max_sell_volume)
        )
        {
            log = std::format( "max buy volume {} max sell volume {}, volume illegal", _market_info->_symbol_details.find(req.symbol)->second->max_buy_volume, _market_info->_symbol_details.find(req.symbol)->second->max_sell_volume);
            pass = false;
        }
        // 报单价格与涨跌停价检查
        else if (
            (req.limit_price < _market_info->_symbol_details.find(req.symbol)->second->lower_limit_price) ||
            (req.limit_price > _market_info->_symbol_details.find(req.symbol)->second->upper_limit_price)
        )
        {
            log = std::format( "lower limit price {} upper limit price {}, price illegal", _market_info->_symbol_details.find(req.symbol)->second->lower_limit_price, _market_info->_symbol_details.find(req.symbol)->second->upper_limit_price);
            pass = false;
        }
        // 重复报单监测和阈值(放在最后检查)
        else if (
            std::find_if(
                _trade_info->_order_data.begin(),
                _trade_info->_order_data.end(),
                [&req](const auto& order) {return order.order_req == req;}
            ) != _trade_info->_order_data.end()
        )
        {
            NANO_LOG(NOTICE, "repeat order detected: %s", data_type::to_json(req).dump(4).c_str());
            if (_risk_indicators->daily_repeat_order_num + 1 > _thresholds->daily_repeat_order_num)
            {
                log = std::format("repeat order num({}) exceed max num({})!",_risk_indicators->daily_repeat_order_num + 1, _thresholds->daily_repeat_order_num);
                pass = false;
            }
            else ++(_risk_indicators->daily_repeat_order_num);
        }
        if (!pass)
        {
            log = std::format("{}\n{}", log, data_type::to_json(req).dump(4));
            NANO_LOG(WARNING, "%s", log.c_str());
            const auto& symbol = req.symbol;
            _db_writer.insert_async(
                db::table::logs::table_name,
                nullptr,
                _market_info->_trading_day,
                _account_config.account_name,
                symbol.symbol.to_string(),
                symbol.trade_symbol.to_string(),
                magic_enum::enum_name(symbol.exchange),
                magic_enum::enum_name(symbol.product_class),
                nullptr,
                "order_insert",
                log,
                "WARNING",
                util::DateTime::now().strftime(), util::DateTime::now().strftime()
            );
        }
        else ++(_risk_indicators->daily_order_num);
        return pass;
    }
    bool RiskControl::check_order_cancel(data_type::OrderRef order_ref)
    {
        std::string log;
        bool pass = true;
        if (!_is_trading)
        {
            log = "trading stopped! order cancel failed";
            pass = false;
        }
        if (order_ref >= _trade_info->_order_data.size())
        {
            log = std::format("order ref {} not found!", order_ref);
            pass = false;
        }
        if (_risk_indicators->daily_cancel_num + 1 > _thresholds->daily_cancel_num)
        {
            log = std::format("order ref {} daily cancel num({}) exceed max num({})!", order_ref, _risk_indicators->daily_cancel_num + 1, _thresholds->daily_cancel_num);
            pass = false;
        }
        if (
            const auto& order = _trade_info->_order_data[order_ref];
            order.is_finished()
        )
        {
            log = std::format("order ref {} order finished!", order_ref);
            pass = false;
        }
        if (!pass)
        {
            log = std::format("{}\n{}", log, order_ref);
            NANO_LOG(WARNING, "%s", log.c_str());
            if (order_ref >= _trade_info->_order_data.size())
            {
                _db_writer.insert_async(
                    db::table::logs::table_name,
                    nullptr,
                    _market_info->_trading_day,
                    _account_config.account_name,
                    nullptr, nullptr, nullptr, nullptr,
                    order_ref,
                    "order_cancel",
                    log,
                    "WARNING",
                    util::DateTime::now().strftime(), util::DateTime::now().strftime()
                );
            }
            else
            {
                const auto& symbol = _trade_info->_order_data[order_ref].order_req.symbol;
                _db_writer.insert_async(
                    db::table::logs::table_name,
                    nullptr,
                    _market_info->_trading_day,
                    _account_config.account_name,
                    symbol.symbol.to_string(),
                    symbol.trade_symbol.to_string(),
                    magic_enum::enum_name(symbol.exchange),
                    magic_enum::enum_name(symbol.product_class),
                    order_ref,
                    "order_cancel",
                    log,
                    "WARNING",
                    util::DateTime::now().strftime(), util::DateTime::now().strftime()
                );
            }
        }
        else ++(_risk_indicators->daily_cancel_num);
        return pass;
    }
    bool RiskControl::check_handle_tick(const data_type::TickData& data)
    {
        if (!_is_trading)
        {
            NANO_LOG(WARNING, "trading not started! %s", data_type::to_json(data).dump(4).c_str());
            return false;
        }
        if (!_market_info)
        {
            NANO_LOG(WARNING, "market info not inited! %s", data_type::to_json(data).dump(4).c_str());
            return false;
        }
        if (!_market_info->_last_tick_data.contains(data.symbol))
        {
            NANO_LOG(WARNING, "unsubscribed symbol %s handle tick", data_type::to_json(data).dump(4).c_str());
            return false;
        }
        return true;
    }
    bool RiskControl::check_handle_bar(const data_type::BarData& data)
    {
        if (!_is_trading)
        {
            NANO_LOG(WARNING, "trading not started! %s", data_type::to_json(data).dump(4).c_str());
            return false;
        }
        if (!_market_info)
        {
            NANO_LOG(WARNING, "market info not inited! %s", data_type::to_json(data).dump(4).c_str());
            return false;
        }
        if (!_market_info->_last_tick_data.contains(data.symbol))
        {
            NANO_LOG(WARNING, "unsubscribed symbol %s handle bar", data_type::to_json(data).dump(4).c_str());
            return false;
        }
        return true;
    }
    bool RiskControl::check_handle_trade(const data_type::TradeData& data)
    {
        std::string log;
        bool pass = true;
        if (!_is_trading)
        {
            log = "trading not started!";
            pass = false;
        }
        else if (_trade_info == nullptr)
        {
            log = "trading not inited!";
            pass = false;
        }
        else if (_trade_info->_order_data.size() <= data.order_ref)
        {
            log = std::format("order data buff size {} order ref {}", _trade_info->_order_data.size(), data.order_ref);
            pass = false;
        }
        else if (_trade_info->_trade_data.size() <= data.order_ref)
        {
            log = std::format("trade data buff size {} order ref {}", _trade_info->_trade_data.size(), data.order_ref);
            pass = false;
        }
        else if (_trade_info->_order_data[data.order_ref].trading_day != _market_info->_trading_day)
        {
            log = std::format("order data invalid! order ref %i ", data.order_ref);
            pass = false;
        }
        else if (data.trading_day != _market_info->_trading_day)
        {
            log = "data invalid!";
            pass = false;
        }
        if (!pass) NANO_LOG(WARNING, "%s\n%s", log.c_str(), data_type::to_json(data).dump(4).c_str());
        return pass;
    }
    bool RiskControl::check_handle_cancel(const data_type::CancelData& data)
    {
        std::string log;
        bool pass = true;
        if (!_is_trading)
        {
            log = "trading not started!";
            pass = false;
        }
        else if (_trade_info == nullptr)
        {
            log = "trade info not inited!";
            pass = false;
        }
        else if (_trade_info->_order_data.size() <= data.order_ref)
        {
            log = std::format("order data size {} order ref {}", _trade_info->_order_data.size(), data.order_ref);
            pass = false;
        }
        else if (_trade_info->_order_data[data.order_ref].trading_day != _market_info->_trading_day)
        {
            log = std::format("order data invalid! order ref {}, order trading day {} market trading day {}", data.order_ref, _trade_info->_order_data[data.order_ref].trading_day,  _market_info->_trading_day);
            pass = false;
        }
        else if (data.trading_day != _market_info->_trading_day)
        {
            log = std::format("trade data invalid! order ref {}, trade trading day {} market trading day {}", data.order_ref, data.trading_day,  _market_info->_trading_day);
            pass = false;
        }
        else {}
        if (!pass) NANO_LOG(WARNING, "%s\n%s", log.c_str(), data_type::to_json(data).dump(4).c_str());
        return pass;
    }
    bool RiskControl::check_handle_error(const data_type::OrderError& data)
    {
        std::string log;
        bool pass = true;
        if (!_is_trading)
        {
            log = "trading not started!";
            pass = false;
        }
        else if (_trade_info == nullptr)
        {
            log = "trade info not inited!";
            pass = false;
        }
        else if (_trade_info->_order_data.size() <= data.order_ref)
        {
            log = std::format("order data size {} order ref {}", _trade_info->_order_data.size(), data.order_ref);
            pass = false;
        }
        else if (_trade_info->_order_data[data.order_ref].trading_day != _market_info->_trading_day)
        {
            log = std::format("order data invalid! order ref {}", data.order_ref);
            pass = false;
        }
        else if (data.trading_day != _market_info->_trading_day)
        {
            log = "data invalid!";
            pass = false;
        }
        else {}
        if (!pass) NANO_LOG(WARNING, "%s\n%s", log.c_str(), data_type::to_json(data).dump(4).c_str());
        return pass;
    }
};