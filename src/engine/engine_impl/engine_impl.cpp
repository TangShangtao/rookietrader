#include "engine_impl/engine_impl.h"
#include "util/db.h"
#include "adapter/adapter.h"
#include "algo/algo.h"
#include "util/datetime.h"
#include <chrono>
#include <unordered_set>
#include <filesystem>
#include <numeric>
#include <magic_enum/magic_enum.hpp>
#include <any>
#include "util/logger.h"
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
        _busy_worker = std::make_unique<std::jthread>([this](const std::stop_token& stop_token){working_loop(stop_token);});
        _md_adapter = adapter::create_md_adapter(
            {
                [this](data_type::TickData&& data){_event_loop->push_event(event::EventType::EVENT_TICK_DATA, data);},
                [this](){_event_loop->push_event(event::EventType::EVENT_MD_DISCONNECTED, std::any());},
            },
            _config.md_adapter_config
        );
        if (!_md_adapter) throw std::runtime_error(std::format("create md gateway failed!"));
        _td_adapter = adapter::create_td_adapter(
            {
                [this](data_type::TradeData&& data){_event_loop->push_event(event::EventType::EVENT_TRADE_DATA, data);},
                [this](data_type::CancelData&& data){_event_loop->push_event(event::EventType::EVENT_CANCEL_DATA, data);},
                [this](data_type::OrderError&& data){_event_loop->push_event(event::EventType::EVENT_ORDER_ERROR, data);},
                [this](){_event_loop->push_event(event::EventType::EVENT_TD_DISCONNECTED, std::any());},
            },
            _config.td_adapter_config
        );
        if (!_td_adapter) throw std::runtime_error(std::format("create td gateway failed!"));
        _oms = std::make_unique<OMS>(_config.account_config, _db_writer);
        _risk_control = std::make_unique<RiskControl>(_is_trading, _config.account_config, _config.risk_control_config, _db_writer);
        _context = std::make_unique<TradingContext>();
        _event_loop->register_handler(
            event::EventType::EVENT_MD_DISCONNECTED,
            [this] (const std::any& event_data)
            {
                RK_LOG_WARN("gateway market front disconnected, reconnecting...");
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
                _md_adapter->logout();
                if (!_md_adapter->login() || !init_market_info())
                {
                    RK_LOG_WARN("gateway market front reconnect failed!");
                    _event_loop->push_event(event::EventType::EVENT_MD_DISCONNECTED, std::any());
                    return;
                }
                RK_LOG_WARN("gateway market front reconnected");
            }
        );
        _event_loop->register_handler(
            event::EventType::EVENT_TD_DISCONNECTED,
            [this] (const std::any& event_data)
            {
                RK_LOG_WARN("gateway trade front disconnected, reconnecting...");
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
                if (!_td_adapter->login() || !init_trade_info())
                {
                    RK_LOG_WARN("gateway trade front reconnect failed!");
                    _event_loop->push_event(event::EventType::EVENT_TD_DISCONNECTED, std::any());
                    return;
                }
                RK_LOG_WARN("gateway trade front reconnected");
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
            RK_LOG_INFO("register strategy {} before trading", strategy_id);
            return;
        };
        // 盘中注册, 立即订阅行情
        RK_LOG_INFO("register strategy {} during trading", strategy_id);
        auto unsubscribed_symbols = init_strategy(strategy_id);
        if (!unsubscribed_symbols.empty())
        {
            if (!_md_adapter->subscribe(_subscribed_symbols))
            {
                RK_LOG_ERROR("strategy_id {} subscribe failed", strategy_id);
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
        RK_LOG_INFO("user {} login trade...", _config.td_adapter_config.user_id.c_str());
        if (!_td_adapter->login())
        {
            RK_LOG_ERROR("login trade failed!");
            return false;
        }
        RK_LOG_INFO("user {} login market...", _config.md_adapter_config.user_id.c_str());
        if (!_md_adapter->login())
        {
            RK_LOG_ERROR("login market failed!");
            return false;
        }
        RK_LOG_INFO("init market info...");
        if (!init_market_info())
        {
            RK_LOG_ERROR("init market info failed, start trading failed!");
            return false;
        }
        RK_LOG_INFO("init trade info...");
        if (!init_trade_info())
        {
            RK_LOG_ERROR("init trade info failed, start trading failed!");
            return false;
        }
        RK_LOG_INFO("trading_day {} start trading...", _market_info->_trading_day);
        _is_trading = true;
        return true;
    }
    void EngineImpl::stop_trading()
    {
        RK_LOG_INFO("stop trading...");
        if (!_is_trading) return;
        _is_trading = false;
        _td_adapter->logout();
        _md_adapter->logout();
        // TODO 落库
        _market_info = nullptr;
        _trade_info = nullptr;

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
            _td_adapter->order_insert(order_ref, req);
            return order_ref;
        }
        return std::nullopt;
    }
    bool EngineImpl::order_cancel(uint32_t strategy_id, data_type::OrderRef order_ref)
    {
        if (_risk_control->check_order_cancel(order_ref))
        {
            _oms->order_cancel(order_ref);
            _td_adapter->order_cancel(order_ref);
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
                RK_LOG_WARN("{}", std::format("create algo return nullptr, {}", data_type::to_json(req).dump(4)).c_str());
                return;
            }
            auto strategy_id = register_strategy(algo);
            algo->set_strategy_id(strategy_id);
            _algos[req.symbol] = {req.algo_name.to_string(), algo};
        }
        _event_loop->push_event(event::EventType::EVENT_ALGO_REQ, req);
    }

    bool EngineImpl::init_trade_info()
    {
        RK_LOG_INFO("query position...");
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 避免柜台查询流控
        auto position_data_opt = _td_adapter->query_position_data();
        if (!position_data_opt)
        {
            RK_LOG_ERROR("query position data failed!");
            return false;
        }
        std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::PositionData>> position_data{};
        for (auto& [key, val] : position_data_opt.value())
        {
            auto it = _market_info->_symbol_details.find(key);
            if (it == _market_info->_symbol_details.end()) continue;
            auto& symbol = _market_info->_symbol_details.find(key)->first;
            val->symbol = symbol;
            position_data[symbol] = std::move(val);
        }
        for (auto& [key, val] : _market_info->_symbol_details)
        {
            if (position_data.find(key) == position_data.end())
            {
                position_data[key] = std::make_shared<data_type::PositionData>(key);
            }
        }
        RK_LOG_INFO("query position success! symbol num: {}", position_data.size());
        RK_LOG_INFO("query order...");
        auto order_data_opt = _td_adapter->query_order_data();
        if (!order_data_opt)
        {
            RK_LOG_ERROR("query order data failed!");
            return false;
        }
        std::vector<data_type::OrderData> order_data{};
        for (auto& val : order_data_opt.value())
        {
            auto it = _market_info->_symbol_details.find(val.order_req.symbol);
            if (it == _market_info->_symbol_details.end()) continue;
            auto& symbol = _market_info->_symbol_details.find(val.order_req.symbol)->first;
            val.order_req.symbol = symbol;
            order_data.emplace_back(std::move(val));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 避免柜台查询流控
        RK_LOG_INFO("query order success! order num: {}", order_data.size());
        RK_LOG_INFO("query trade...");
        auto trade_data_opt = _td_adapter->query_trade_data();
        if (!trade_data_opt)
        {
            RK_LOG_ERROR("query trade data failed!");
            return false;
        }
        auto trade_data = trade_data_opt.value();
        RK_LOG_INFO("query trade success! trade num: {}", std::accumulate(trade_data.begin(), trade_data.end(), 0, [](size_t sum, const auto& row) { return sum + row.size(); }));
        RK_LOG_INFO("query account...");
        auto account_data_opt = _td_adapter->query_account_data();
        if (!account_data_opt)
        {
            RK_LOG_ERROR("query account data failed!");
            return false;
        }
        auto account_data = account_data_opt.value();
        RK_LOG_INFO("query account success! account data:\n{}", data_type::to_json(account_data).dump(4).c_str());
        auto trade_info = std::make_shared<TradeInfo>(
            _config.account_config.account_name,
            std::move(position_data),
            std::move(order_data),
            std::move(trade_data),
            std::move(account_data)
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
            market_info->_trading_day = _td_adapter->query_trading_day();
            // 查合约
            RK_LOG_INFO("query symbol detail...");
            auto symbol_detail = _md_adapter->query_symbol_detail();
            if (!symbol_detail)
            {
                RK_LOG_ERROR("query symbol detail failed!");
                return false;
            }
            RK_LOG_INFO("query symbol success, symbol num {}", symbol_detail.value().size());
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
        RK_LOG_INFO("subscribing market data...");
        if (_subscribed_symbols.empty())
        {
            RK_LOG_WARN("no symbol subscribed");
        }
        else
        {
            if (!_md_adapter->subscribe(_subscribed_symbols))
            {
                RK_LOG_ERROR("subscribe failed!");
                return false;
            }
            RK_LOG_INFO("subscribe success, symbol num {}", _subscribed_symbols.size());
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
            RK_LOG_ERROR("{}", error.c_str());
            throw std::runtime_error(error);
        }
        std::unordered_set<data_type::Symbol> ret;
        for (const auto& symbol : _strategies[strategy_id]->on_init(_market_info->_trading_day))
        {
            if (!_market_info->_symbol_details.contains(symbol))
            {
                RK_LOG_WARN("strategy_id {} symbol {} not found in symbol details, pass subscribe", strategy_id, symbol.symbol.c_str());
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
    void EngineImpl::working_loop(const std::stop_token& stop_token)
    {
        while (!stop_token.stop_requested())
        {
            auto begin = std::chrono::system_clock::now();
            if (_is_trading)
            {
                _event_loop->handle_event();
            }
            auto use_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - begin);
            auto duration = std::chrono::microseconds(100); // TODO 通过配置
            if (use_time < duration)
            {
                std::this_thread::sleep_for(duration - use_time);
            }
        }
        RK_LOG_INFO("engine stopped");
    }

};