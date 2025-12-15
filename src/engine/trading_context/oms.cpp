//
// Created by root on 2025/9/29.
//
#include "oms.h"
#include "engine_impl/engine_impl.h"
#include "serialization.h"
#include "util/datetime.h"
#include <NanoLogCpp17.h>
#include <magic_enum/magic_enum.hpp>
using namespace NanoLog::LogLevels;
namespace rk
{
    OMS::OMS(
        const config_type::AccountConfig& account_config,
        db::Executor& db_writer
    )
    : _account_config(account_config), _db_writer(db_writer)
    {

    }
    void OMS::set_trade_info(std::shared_ptr<TradeInfo> trade_info)
    {
        _trade_info = std::move(trade_info);
        if (_market_info)
        {
            for (const auto& [symbol, detail] : _market_info->_symbol_details)
            {
                if (_trade_info->_position_data.find(symbol) == _trade_info->_position_data.end())
                {
                    _trade_info->_position_data[symbol] = std::make_shared<data_type::PositionData>();
                }
            }
        }
        if (_trade_info->_trade_data.size() < _trade_info->_order_data.size())
        {
            _trade_info->_trade_data.resize(_trade_info->_order_data.size());
        }

    }
    void OMS::set_market_info(std::shared_ptr<MarketInfo> market_info)
    {
        _market_info = std::move(market_info);
        for (const auto& [symbol, detail] : _market_info->_symbol_details)
        {
            if (_trade_info && _trade_info->_position_data.find(symbol) == _trade_info->_position_data.end())
            {
                _trade_info->_position_data[symbol] = std::make_shared<data_type::PositionData>();
            }
            _market_info->_last_tick_data[symbol] = std::make_shared<data_type::TickData>();
        }

    }
    data_type::OrderRef OMS::order_insert(const data_type::OrderReq& req)
    {
        auto& position = _trade_info->_position_data[req.symbol];
        auto order_ref = static_cast<data_type::OrderRef>(_trade_info->_order_data.size());
        _trade_info->_order_data.emplace_back(data_type::OrderData{
            order_ref,
            req,
            _market_info->_trading_day, util::DateTime::now(),
            // data_type::OrderStatus::QUEUEING,
            0, req.volume, 0
        });
        _trade_info->_trade_data.emplace_back(std::vector<data_type::TradeData>{});
        switch (req.direction)
        {
            case data_type::Direction::LONG:
            {
                switch (req.offset)
                {
                    case data_type::Offset::OPEN:
                    {
                        position->long_position.pending += req.volume;
                        break;
                    }
                    case data_type::Offset::CLOSE:
                    case data_type::Offset::CLOSE_TD:
                    case data_type::Offset::CLOSE_YD:
                    {
                        position->short_position.frozen += req.volume;
                        break;
                    }
                    default:
                    {
                        NANO_LOG(ERROR, "unknown offset %s", req.symbol.symbol.c_str());
                        break;
                    }
                }
                break;
            }
            case data_type::Direction::SHORT:
            {
                switch (req.offset)
                {
                    case data_type::Offset::OPEN:
                    {
                        position->short_position.pending += req.volume;
                        break;
                    }
                    case data_type::Offset::CLOSE:
                    case data_type::Offset::CLOSE_TD:
                    case data_type::Offset::CLOSE_YD:
                    {
                        position->long_position.frozen += req.volume;
                        break;
                    }
                    default:
                    {
                        NANO_LOG(ERROR, "unknown offset %s", req.symbol.symbol.c_str());
                        break;
                    }
                }
                break;
            }
            default:
            {
                NANO_LOG(ERROR, "unknown direction %s", req.symbol.symbol.c_str());
            }
        }
        auto log = std::format("order_insert: {}", data_type::to_json(_trade_info->_order_data[order_ref]).dump(4));
        NANO_LOG(NOTICE, "%s", log.c_str());
        const auto& symbol = req.symbol;
        _db_writer.insert_async(
            db::table::logs::table_name,
            nullptr,
            _market_info->_trading_day,
            _account_config.account_name,
            symbol.symbol,
            symbol.trade_symbol,
            magic_enum::enum_name(symbol.exchange),
            magic_enum::enum_name(symbol.product_class),
            order_ref,
            "order_insert",
            log,
            "NOTICE",
            util::DateTime::now().strftime(), util::DateTime::now().strftime()
        );
        return order_ref;
    }

    void OMS::order_cancel(data_type::OrderRef order_ref)
    {
        auto log = std::format("order_cancel: {}", data_type::to_json(_trade_info->_order_data[order_ref]).dump(4));
        NANO_LOG(NOTICE, "%s", log.c_str());
        const auto& symbol = _trade_info->_order_data[order_ref].order_req.symbol;
        _db_writer.insert_async(
            db::table::logs::table_name,
            nullptr,
            _market_info->_trading_day,
            _account_config.account_name,
            symbol.symbol,
            symbol.trade_symbol,
            magic_enum::enum_name(symbol.exchange),
            magic_enum::enum_name(symbol.product_class),
            order_ref,
            "order_cancel",
            log,
            "NOTICE",
            util::DateTime::now().strftime(), util::DateTime::now().strftime()
        );
        return;
    }
	void OMS::handle_tick(const data_type::TickData& data)
	{
        *_market_info->_last_tick_data[data.symbol] = data;
	}
    void OMS::handle_bar(const data_type::BarData& data)
    {

    }
	void OMS::handle_trade(const data_type::TradeData& data)
	{
        _trade_info->_trade_data[data.order_ref].emplace_back(data);
        auto& order_data = _trade_info->_order_data[data.order_ref];
        order_data.traded_volume += data.trade_volume;
        order_data.remain_volume -= data.trade_volume;

        const auto& order_req = order_data.order_req;
        auto& position = _trade_info->_position_data[order_req.symbol];
        switch (order_req.direction)
        {
            case data_type::Direction::LONG:
            {
                switch (order_req.offset)
                {
                    case data_type::Offset::OPEN:
                    {
                        position->long_position.position += data.trade_volume;
                        position->long_position.pending -= data.trade_volume;
                        break;
                    }
                    case data_type::Offset::CLOSE:
                    case data_type::Offset::CLOSE_TD:
                    case data_type::Offset::CLOSE_YD:
                    {
                        position->short_position.position -= data.trade_volume;
                        position->short_position.frozen -= data.trade_volume;
                        break;
                    }
                    default:
                    {
                        NANO_LOG(ERROR, "unknown offset %s", order_req.symbol.symbol.c_str());
                        break;
                    }
                }
                break;
            }
            case data_type::Direction::SHORT:
            {
                switch (order_req.offset)
                {
                    case data_type::Offset::OPEN:
                    {
                        position->short_position.position += data.trade_volume;
                        position->short_position.pending -= data.trade_volume;
                        break;
                    }
                    case data_type::Offset::CLOSE:
                    case data_type::Offset::CLOSE_TD:
                    case data_type::Offset::CLOSE_YD:
                    {
                        position->long_position.position -= data.trade_volume;
                        position->long_position.frozen -= data.trade_volume;
                        break;
                    }
                    default:
                    {
                        NANO_LOG(ERROR, "unknown offset %s", order_req.symbol.symbol.c_str());
                        break;
                    }
                }
                break;
            }
            default:
            {
                NANO_LOG(ERROR, "unknown direction %s", order_req.symbol.symbol.c_str());
                break;
            }
        }
        auto log = std::format("handle_trade: {}", data_type::to_json(data).dump(4));
        NANO_LOG(NOTICE, "%s", log.c_str());
        const auto& symbol = _trade_info->_order_data[data.order_ref].order_req.symbol;
        _db_writer.insert_async(
            db::table::logs::table_name,
            nullptr,
            _market_info->_trading_day,
            _account_config.account_name,
            symbol.symbol,
            symbol.trade_symbol,
            magic_enum::enum_name(symbol.exchange),
            magic_enum::enum_name(symbol.product_class),
            data.order_ref,
            "handle_trade",
            log,
            "NOTICE",
            util::DateTime::now().strftime(), util::DateTime::now().strftime()
        );
        return;
	}
    void OMS::handle_cancel(const data_type::CancelData& data)
    {
        auto& order_data = _trade_info->_order_data[data.order_ref];
        order_data.remain_volume -= data.cancel_volume;
        order_data.canceled_volume += data.cancel_volume;

        const auto& order_req = order_data.order_req;
        auto& position = _trade_info->_position_data[order_req.symbol];
        switch (order_req.direction)
        {
            case data_type::Direction::LONG:
            {
                switch (order_req.offset)
                {
                    case data_type::Offset::OPEN:
                    {
                        position->long_position.pending -= data.cancel_volume;
                        break;
                    }
                    case data_type::Offset::CLOSE:
                    case data_type::Offset::CLOSE_TD:
                    case data_type::Offset::CLOSE_YD:
                    {
                        position->short_position.frozen -= data.cancel_volume;
                        break;
                    }
                    default:
                    {
                        NANO_LOG(ERROR, "unknown offset %s", order_req.symbol.symbol.c_str());
                        break;
                    }
                }
                break;
            }
            case data_type::Direction::SHORT:
            {
                switch (order_req.offset)
                {
                    case data_type::Offset::OPEN:
                    {
                        position->short_position.pending -= data.cancel_volume;
                        break;
                    }
                    case data_type::Offset::CLOSE:
                    case data_type::Offset::CLOSE_TD:
                    case data_type::Offset::CLOSE_YD:
                    {
                        position->long_position.frozen -= data.cancel_volume;
                        break;
                    }
                    default:
                    {
                        NANO_LOG(ERROR, "unknown offset %s", order_req.symbol.symbol.c_str());
                        break;
                    }
                }
                break;
            }
            default:
            {
                NANO_LOG(ERROR, "unknown direction %s", order_req.symbol.symbol.c_str());
                break;
            }
        }
        auto log = std::format("handle_cancel: {}", data_type::to_json(data).dump(4));
        NANO_LOG(NOTICE, "%s", log.c_str());
        const auto& symbol = _trade_info->_order_data[data.order_ref].order_req.symbol;
        _db_writer.insert_async(
            db::table::logs::table_name,
            nullptr,
            _market_info->_trading_day,
            _account_config.account_name,
            symbol.symbol,
            symbol.trade_symbol,
            magic_enum::enum_name(symbol.exchange),
            magic_enum::enum_name(symbol.product_class),
            data.order_ref,
            "handle_cancel",
            log,
            "NOTICE",
            util::DateTime::now().strftime(), util::DateTime::now().strftime()
        );
    }
    void OMS::handle_error(const data_type::OrderError& data)
    {
        auto& order_data = _trade_info->_order_data[data.order_ref];
        order_data.remain_volume = 0;
        order_data.canceled_volume = 0;
        order_data.traded_volume = 0;
        const auto& order_req = order_data.order_req;
        auto& position = _trade_info->_position_data[order_req.symbol];
        switch (data.error_type)
        {
            case data_type::ErrorType::ORDER_INSERT_ERROR:
            {
                switch (order_req.direction)
                {
                    case data_type::Direction::LONG:
                    {
                        switch (order_req.offset)
                        {
                            case data_type::Offset::OPEN:
                            {
                                position->long_position.pending -= order_req.volume;
                                break;
                            }
                            case data_type::Offset::CLOSE:
                            case data_type::Offset::CLOSE_TD:
                            case data_type::Offset::CLOSE_YD:
                            {
                                position->short_position.frozen -= order_req.volume;
                                break;
                            }
                            default:
                            {
                                NANO_LOG(ERROR, "unknown offset %s", order_req.symbol.symbol.c_str());
                                break;
                            }
                        }
                        break;
                    }
                    case data_type::Direction::SHORT:
                    {
                        switch (order_req.offset)
                        {
                            case data_type::Offset::OPEN:
                            {
                                position->short_position.pending -= order_req.volume;
                                break;
                            }
                            case data_type::Offset::CLOSE:
                            case data_type::Offset::CLOSE_TD:
                            case data_type::Offset::CLOSE_YD:
                            {
                                position->long_position.frozen -= order_req.volume;
                                break;
                            }
                            default:
                            {
                                NANO_LOG(ERROR, "unknown offset %s", order_req.symbol.symbol.c_str());
                                break;
                            }
                        }
                        break;
                    }
                    default:
                    {
                        NANO_LOG(ERROR, "unknown direction %s", order_req.symbol.symbol.c_str());
                        break;
                    }
                }
                break;
            }
            case data_type::ErrorType::ORDER_CANCEL_ERROR:
            {
                break;
            }
            default:
            {
                NANO_LOG(ERROR, "unknown error type %s", order_req.symbol.symbol.c_str());
                break;
            }
        }
        auto log = std::format("handle_error: {}", data_type::to_json(data).dump(4));
        NANO_LOG(NOTICE, "%s", log.c_str());
        const auto& symbol = _trade_info->_order_data[data.order_ref].order_req.symbol;
        _db_writer.insert_async(
            db::table::logs::table_name,
            nullptr,
            _market_info->_trading_day,
            _account_config.account_name,
            symbol.symbol,
            symbol.trade_symbol,
            magic_enum::enum_name(symbol.exchange),
            magic_enum::enum_name(symbol.product_class),
            data.order_ref,
            "handle_error",
            log,
            "NOTICE",
            util::DateTime::now().strftime(), util::DateTime::now().strftime()
        );
    }


};