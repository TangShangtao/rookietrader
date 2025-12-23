#include "engine_impl/engine_impl.h"
#include "db.h"
#include "gateway/gateway.h"
#include "algo/algo.h"
#include "util/datetime.h"
#include "trading_context/trading_context.h"
#include "trading_context/oms.h"
#include "trading_context/risk_control.h"
#include "serialization.h"
#include <chrono>
#include <unordered_set>
#include <filesystem>
#include <numeric>
#include <nlohmann/json.hpp>
#include <NanoLogCpp17.h>
#include <magic_enum/magic_enum.hpp>

using namespace NanoLog::LogLevels;
namespace rk
{
    EngineImpl::EngineImpl(config_type::EngineConfig config)
        :
        _config(std::move(config)),
        _is_trading(false),
        _event_loop(std::make_shared<event::EventLoop>()),
        _db_reader(std::format(
            "host={} port={} dbname={} user={} password={}",
            _config.db_config.ip, _config.db_config.port, _config.db_config.database, _config.db_config.user, _config.db_config.password
        )),
        _db_writer(_config.db_config)
    {
        std::filesystem::create_directories(_config.log_config.log_file_parent_path);
        NanoLog::preallocate();
        NanoLog::setLogFile(
            std::format(
                "{}/rkengine_{}_{}.nanolog",
                _config.log_config.log_file_parent_path,
                _config.account_config.account_name,
                util::DateTime::now().strftime("%Y-%m-%d_%H:%M:%S")
            )
            .c_str()
        );
        NanoLog::setLogLevel(
            magic_enum::enum_cast<LogLevel>(_config.log_config.log_level, magic_enum::case_insensitive).value_or(LogLevel::NOTICE)
        );
        _busy_worker = std::make_unique<std::jthread>([this](std::stop_token st) -> void
        {
            NanoLog::preallocate();
            while (!st.stop_requested())
            {
                auto begin = std::chrono::system_clock::now();
                this->execute();
                auto use_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - begin);
                auto duration = std::chrono::microseconds(100); // TODO 通过配置
                if (use_time < duration)
                {
                   std::this_thread::sleep_for(duration - use_time);
                }
            }
            NANO_LOG(NOTICE, "engine stopped");
        });
        _gateway = gateway::create_gateway(_event_loop, _config.gateway_config);
        if (!_gateway) throw std::runtime_error(std::format("create gateway failed!"));
        _oms = std::make_unique<OMS>(_config.account_config, _db_writer);
        _risk_control = std::make_unique<RiskControl>(_is_trading, _config.account_config, config.risk_control_config, _db_writer);
        _context = std::make_unique<TradingContext>();
        _event_loop->register_handler(
            event::EventType::EVENT_MD_DISCONNECTED,
            [this] (const std::any& event_data)
            {
                NANO_LOG(WARNING, "gateway market front disconnected, reconnecting...");
                _db_writer.insert_async(
                    db::table::logs::table_name,nullptr,
                    _market_info->_trading_day,
                    _config.account_config.account_name,
                    nullptr, nullptr, nullptr, nullptr, nullptr,
                    magic_enum::enum_name(event::EventType::EVENT_MD_DISCONNECTED),
                    "",
                    "WARNING",
                    util::DateTime::now().strftime(), util::DateTime::now().strftime()
                );
                _gateway->logout_market();
                if (!_gateway->login_market() || !init_market_info())
                {
                    NANO_LOG(WARNING, "gateway market front reconnect failed!");
                    _event_loop->push_event(event::EventType::EVENT_MD_DISCONNECTED, std::any());
                    return;
                }
                NANO_LOG(WARNING, "gateway market front reconnected");
            }
        );
        _event_loop->register_handler(
            event::EventType::EVENT_TD_DISCONNECTED,
            [this] (const std::any& event_data)
            {
                NANO_LOG(WARNING, "gateway trade front disconnected, reconnecting...");
                _db_writer.insert_async(
                    db::table::logs::table_name,nullptr,
                    _market_info->_trading_day,
                    _config.account_config.account_name,
                    nullptr, nullptr, nullptr, nullptr, nullptr,
                    magic_enum::enum_name(event::EventType::EVENT_TD_DISCONNECTED),
                    "",
                    "WARNING",
                    util::DateTime::now().strftime(), util::DateTime::now().strftime()
                );
                if (!_gateway->login_trade() || !init_trade_info())
                {
                    NANO_LOG(WARNING, "gateway trade front reconnect failed!");
                    _event_loop->push_event(event::EventType::EVENT_TD_DISCONNECTED, std::any());
                    return;
                }
                NANO_LOG(WARNING, "gateway trade front reconnected");
            }
        );
        _event_loop->register_handler(
            event::EventType::EVENT_TICK_DATA,
            [this] (const std::any& event_data)
            {
                const auto& data = std::any_cast<const data_type::TickData&>(event_data);
                if (!_risk_control->check_handle_tick(data)) return;
                _oms->handle_tick(data);
                _context->handle_tick(data);
            }
        );
        _event_loop->register_handler(
            event::EventType::EVENT_TRADE_DATA,
            [this] (const std::any& event_data)
            {
                const auto& data = std::any_cast<const data_type::TradeData&>(event_data);
                if (!_risk_control->check_handle_trade(data)) return;
                _oms->handle_trade(data);
                _context->handle_trade(data);
            }
        );
        _event_loop->register_handler(
            event::EventType::EVENT_CANCEL_DATA,
            [this] (const std::any& event_data)
            {
                const auto& data = std::any_cast<const data_type::CancelData&>(event_data);
                if (!_risk_control->check_handle_cancel(data)) return;
                _oms->handle_cancel(data);
                _context->handle_cancel(data);
            }
        );
        _event_loop->register_handler(
            event::EventType::EVENT_ORDER_ERROR,
            [this] (const std::any& event_data)
            {
                const auto& data = std::any_cast<const data_type::OrderError&>(event_data);
                if (!_risk_control->check_handle_error(data)) return;
                _oms->handle_error(data);
                _context->handle_error(data);
            }
        );
        _event_loop->register_handler(
            event::EventType::EVENT_ALGO_REQ,
            [this] (const std::any& event_data)
            {
                const auto& data = std::any_cast<const data_type::AlgoReq&>(event_data);
                auto& [_, algo] = _algos[data.symbol];
                algo->on_algo_req(data);
            }

        );
    }
    EngineImpl::EngineImpl(std::string_view config_file_path)
        : EngineImpl(config_type::load_engine_config(config_file_path))
    {}
    EngineImpl::~EngineImpl()
    {
        if (_is_trading)
        {
            stop_trading();
        }
    }
    void EngineImpl::register_strategy(uint32_t strategy_id, std::shared_ptr<interface::Strategy> strategy_)
    {
        if (strategy_id >= _strategies.size()) _strategies.push_back(std::move(strategy_));
        else _strategies[strategy_id] = std::move(strategy_);
        // 盘前注册, 等待init_market_info统一订阅行情
        if (!_is_trading)
        {
            NANO_LOG(NOTICE, "register strategy %i before trading", strategy_id);
            return;
        };
        // 盘中注册, 立即订阅行情
        NANO_LOG(NOTICE, "register strategy %i during trading", strategy_id);
        auto unsubscribed_symbols = init_strategy(strategy_id);
        if (!unsubscribed_symbols.empty())
        {
            if (!_gateway->subscribe(_subscribed_symbols))
            {
                NANO_LOG(ERROR, "strategy_id %i subscribe failed", strategy_id);
            }
        }
        _subscribed_symbols.merge(unsubscribed_symbols);
    }
    uint32_t EngineImpl::register_strategy(std::shared_ptr<interface::Strategy> strategy)
    {
        auto strategy_id = _strategies.size();
        register_strategy(strategy_id, std::move(strategy));
        return strategy_id;
    }

    bool EngineImpl::start_trading()
    {
        if (_is_trading) return false;
        if (!_gateway)
        {
            NANO_LOG(ERROR, "gateway is null, start trading failed!");
            return false;
        }
        NANO_LOG(NOTICE, "user %s login trade...", _config.gateway_config.user_id.c_str());
        if (!_gateway->login_trade())
        {
            NANO_LOG(ERROR, "login trade failed!");
            return false;
        }
        NANO_LOG(NOTICE, "user %s login market...", _config.gateway_config.user_id.c_str());
        if (!_gateway->login_market())
        {
            NANO_LOG(ERROR, "login market failed!");
            return false;
        }
        NANO_LOG(NOTICE, "init market info...");
        if (!init_market_info())
        {
            NANO_LOG(ERROR, "init market info failed, start trading failed!");
            return false;
        }
        NANO_LOG(NOTICE, "init trade info...");
        if (!init_trade_info())
        {
            NANO_LOG(ERROR, "init trade info failed, start trading failed!");
            return false;
        }
        NANO_LOG(NOTICE, "trading_day %i start trading...", _market_info->_trading_day);
        _is_trading = true;
        return true;
    }
    void EngineImpl::stop_trading()
    {
        NANO_LOG(NOTICE, "stop trading...");
        if (!_is_trading) return;
        _is_trading = false;
        _gateway->logout_trade();
        _gateway->logout_market();
        // TODO 落库
        _market_info = nullptr;
        _trade_info = nullptr;

    }
    std::string EngineImpl::engine_detail_json() const
    {
        nlohmann::json detail{};
        nlohmann::json trade_info{};
        trade_info["account_name"] = _trade_info->_account_name;
        trade_info["position_data"] = {};
        for (const auto& [symbol, position] : _trade_info->_position_data)
        {
            if (position == nullptr || position->empty()) continue;
            trade_info["position_data"][symbol.symbol.to_string()] = data_type::to_json(*position);
        }
        trade_info["order_data"] = nlohmann::json::array();
        for (const auto& order : _trade_info->_order_data)
        {
            trade_info["order_data"].emplace_back(data_type::to_json(order));
        }
        trade_info["trade_data"] = nlohmann::json::array();
        for (const auto& each_order : _trade_info->_trade_data)
        {
            for (const auto& trade : each_order)
            {
                trade_info["trade_data"].emplace_back(data_type::to_json(trade));
            }
        }
        trade_info["account_data"] = data_type::to_json(_trade_info->_account_data);
        nlohmann::json market_info{};
        market_info["trading_day"] = _market_info->_trading_day;
        market_info["symbol_details"] = {};
        for (const auto& [symbol, data] : _market_info->_symbol_details)
        {
            market_info["symbol_details"][symbol.symbol.to_string()].emplace_back(data_type::to_json(*data));
        }
        market_info["last_tick_data"] = {};
        for (const auto& [symbol, data] : _market_info->_last_tick_data)
        {
            market_info["last_tick_data"][symbol.symbol.to_string()].emplace_back(data_type::to_json(*data));
        }
        detail["trade_info"] = trade_info;
        detail["market_info"] = market_info;
        return detail.dump();
    }
    std::unordered_set<data_type::Symbol> EngineImpl::query_all_symbols() const
    {
        std::unordered_set<data_type::Symbol> set;
        set.reserve(_market_info->_symbol_details.size());
        for (const auto& pair : _market_info->_symbol_details)
        {
            set.insert(pair.first);
        }
        return set;
    }
    std::vector<data_type::PositionData> EngineImpl::query_all_positions() const
    {
        std::vector<data_type::PositionData> vec;
        if (_trade_info != nullptr)
        {
            vec.reserve(_trade_info->_position_data.size());
            for (const auto& [symbol, position_data] : _trade_info->_position_data)
            {
                if (position_data->long_position.empty() && position_data->short_position.empty()) continue;
                vec.emplace_back(*position_data);
            }
        }
        return vec;
    }
    std::shared_ptr<const data_type::TickData> EngineImpl::query_last_tick(const data_type::Symbol& symbol)
    {
        auto it = _market_info->_last_tick_data.find(symbol);
        if (it == _market_info->_last_tick_data.end()) return nullptr;
        return it->second;
    }
    std::shared_ptr<const data_type::SymbolDetail> EngineImpl::query_symbol_detail(const data_type::Symbol& symbol) const
    {
        auto it = _market_info->_symbol_details.find(symbol);
        if (it == _market_info->_symbol_details.end()) return nullptr;
        return it->second;
    }
    std::shared_ptr<const data_type::PositionData> EngineImpl::query_position_data(const data_type::Symbol& symbol) const
    {
        if (_trade_info != nullptr)
        {
            auto it = _trade_info->_position_data.find(symbol);
            if (it == _trade_info->_position_data.end()) return nullptr;
            return it->second;
        }
        return nullptr;
    }
    const data_type::AccountData& EngineImpl::query_account_data() const
    {
        return _trade_info->_account_data;
    }
    const data_type::OrderData& EngineImpl::query_order_data(data_type::OrderRef order_ref) const
    {
        return _trade_info->_order_data[order_ref];
    }
    const std::vector<data_type::TradeData>& EngineImpl::query_trade_data(data_type::OrderRef order_ref) const
    {
        return _trade_info->_trade_data[order_ref];
    }
    std::shared_ptr<const TradeInfo> EngineImpl::query_trade_info() const
    {
        return _trade_info;
    }
    std::optional<data_type::OrderRef> EngineImpl::order_insert(uint32_t strategy_id, const data_type::OrderReq& req)
    {
        if (_risk_control->check_order_insert(req))
        {
            auto order_ref = _oms->order_insert(req);
            _context->order_insert(
                TradeHandler{
                    [strategy_id, this](const data_type::TradeData& data) {_strategies[strategy_id]->on_trade(data);},
                    [strategy_id, this](const data_type::CancelData& data) {_strategies[strategy_id]->on_cancel(data);},
                    [strategy_id, this](const data_type::OrderError& data) {_strategies[strategy_id]->on_error(data);},
                },
                order_ref
            );
            _gateway->order_insert(order_ref, req);
            return order_ref;
        }
        return std::nullopt;
    }
    bool EngineImpl::order_cancel(uint32_t strategy_id, data_type::OrderRef order_ref)
    {
        if (_risk_control->check_order_cancel(order_ref))
        {
            _oms->order_cancel(order_ref);
            _gateway->order_cancel(order_ref);
            return true;
        }
        return false;
    }
    void EngineImpl::algo_insert(const data_type::AlgoReq& req)
    {
        // TODO 本地风控
        if (
            _algos.find(req.symbol) == _algos.end() ||
            std::get<std::string>(_algos[req.symbol]) != req.algo_name
        )
        {
            auto algo = algo::create_algo(*this, req.symbol, req.algo_name.to_string(), req.algo_param_json.to_string());
            if (algo == nullptr)
            {
                NANO_LOG(WARNING, "%s", std::format("create algo return nullptr, {}", data_type::to_json(req).dump(4)).c_str());
                return;
            }
            auto strategy_id = register_strategy(algo);
            algo->set_strategy_id(strategy_id);
            _algos[req.symbol] = {req.algo_name.to_string(), algo};
        }
        _event_loop->push_event(event::EventType::EVENT_ALGO_REQ, req);
    }
    void EngineImpl::execute()
    {
        if (_is_trading)
        {
            _event_loop->handle_event();
        }

    }
    bool EngineImpl::init_trade_info()
    {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 避免柜台查询流控
        auto position_data = _gateway->query_position_data();
        if (!position_data)
        {
            NANO_LOG(ERROR, "query position data failed!");
            return false;
        }
        NANO_LOG(NOTICE, "query position success! symbol num: %lu", position_data->size());
        auto order_data = _gateway->query_order_data();
        if (!order_data)
        {
            NANO_LOG(ERROR, "query order data failed!");
            return false;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 避免柜台查询流控
        NANO_LOG(NOTICE, "query order success! order num: %lu", order_data->size());
        auto trade_data = _gateway->query_trade_data();
        if (!trade_data)
        {
            NANO_LOG(ERROR, "query trade data failed!");
            return false;
        }
        NANO_LOG(NOTICE, "query trade success! trade num: %d", std::accumulate(trade_data->begin(), trade_data->end(), 0, [](size_t sum, const auto& row) { return sum + row.size(); }));
        auto account_data = _gateway->query_account_data();
        if (!account_data)
        {
            NANO_LOG(ERROR, "query account data failed!");
            return false;
        }
        NANO_LOG(NOTICE, "query account success! account data:\n%s", data_type::to_json(*account_data).dump(4).c_str());
        auto trade_info = std::make_shared<TradeInfo>(
            _config.account_config.account_name,
            std::move(position_data.value()),
            std::move(order_data.value()),
            std::move(trade_data.value()),
            std::move(account_data.value())
        );
        _risk_indicators = _risk_control->set_trade_info(trade_info);
        _oms->set_trade_info(trade_info);
        _trade_info = trade_info;
        return true;
    }
    bool EngineImpl::init_market_info()
    {
        // 首次启动
        if (_market_info == nullptr)
        {
            const auto market_info = std::make_shared<MarketInfo>();
            // 切换交易日
            market_info->_trading_day = _gateway->query_trading_day();
            // 查合约
            auto symbol_detail = _gateway->query_symbol_details();
            if (!symbol_detail)
            {
                NANO_LOG(ERROR, "query symbol detail failed!");
                return false;
            }
            NANO_LOG(NOTICE, "query symbol success, symbol num %zu", symbol_detail.value().size());
            market_info->_symbol_details = std::move(symbol_detail.value());
            _oms->set_market_info(market_info);
            _risk_control->set_market_info(market_info);
            _market_info = market_info;
            // 确定订阅合约, 注册行情回调
            _subscribed_symbols.clear();
            for (uint32_t strategy_id = 0; strategy_id < _strategies.size(); ++strategy_id)
            {
                _subscribed_symbols.merge(init_strategy(strategy_id));
            }
        }
        // 断线重连
        else {}
        // TODO 订阅合约按理说应该先查询最新tick, 目前直接赋为空
        auto last_tick_data = std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::TickData>>{};
        for (const auto & symbol : _subscribed_symbols)
        {
            last_tick_data[symbol] = std::make_shared<data_type::TickData>();
        }
        // 向柜台订阅行情
        if (_subscribed_symbols.empty())
        {
            NANO_LOG(WARNING, "no symbol subscribed");
        }
        else
        {
            if (!_gateway->subscribe(_subscribed_symbols))
            {
                NANO_LOG(ERROR, "subscribe failed!");
                return false;
            }
            NANO_LOG(NOTICE, "subscribe success, symbol num %zu", _subscribed_symbols.size());
        }
        return true;
    }
    std::unordered_set<data_type::Symbol> EngineImpl::init_strategy(uint32_t strategy_id)
    {
        if (
            (_strategies.size() <= strategy_id) ||
            (_strategies[strategy_id] == nullptr)
        )
        {
            const auto error = std::format("strategy_id {} not found or nullptr!", strategy_id);
            NANO_LOG(ERROR, "%s", error.c_str());
            throw std::runtime_error(error);
        }
        std::unordered_set<data_type::Symbol> ret;
        for (const auto& symbol : _strategies[strategy_id]->on_init(_market_info->_trading_day))
        {
            if (!_market_info->_symbol_details.contains(symbol))
            {
                NANO_LOG(WARNING, "strategy_id %i symbol %s not found in symbol details, pass subscribe", strategy_id, symbol.symbol.c_str());
                continue;
            }
            _context->subscribe(
                MarketHandler{
                    [strategy_id, this](const data_type::TickData& data){_strategies[strategy_id]->on_tick(data);},
                    [strategy_id, this](const data_type::BarData& data){_strategies[strategy_id]->on_bar(data);}
                },
                symbol
            );
            ret.emplace(std::move(symbol));
        }
        return ret;
    }

};