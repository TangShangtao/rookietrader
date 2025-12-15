#pragma once
#include <thread>
#include <vector>
#include <unordered_set>
#include <memory>
#include <pqxx/pqxx>
#include "config_type.h"
#include "event.h"
#include "interface.h"
#include "db.h"


namespace rk
{
    namespace gateway {class Gateway;}
    namespace algo {class Algo;}
    class OMS;
    class RiskControl;
    class TradingContext;
    struct TradeInfo
    {
        std::string _account_name;
        std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::PositionData>> _position_data;
        std::vector<data_type::OrderData> _order_data;
        std::vector<std::vector<data_type::TradeData>> _trade_data;
        data_type::AccountData _account_data;
    };
    struct MarketInfo
    {
        uint32_t _trading_day = std::stoul(util::DateTime::now().strftime("%Y%m%d"));
        std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>> _symbol_details;
        std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::TickData>> _last_tick_data;
    };
    struct RiskIndicators
    {
        int daily_order_num = 0;
        int daily_cancel_num = 0;
        int daily_repeat_order_num = 0;
    };
    class EngineImpl
    {
    public:
        explicit EngineImpl(config_type::EngineConfig config);
        explicit EngineImpl(std::string_view config_file_path);
        ~EngineImpl();
        // basic
        uint32_t register_strategy(std::shared_ptr<interface::Strategy> strategy);
        void register_strategy(uint32_t strategy_id, std::shared_ptr<interface::Strategy> strategy);
        bool start_trading();
        void stop_trading();
        // 接口在strategy回调函数内调用才线程安全
        // query
        std::string engine_detail_json() const;
        std::unordered_set<data_type::Symbol> query_all_symbols() const;
        std::vector<data_type::PositionData> query_all_positions() const;
        std::shared_ptr<const data_type::TickData> query_last_tick(const data_type::Symbol& symbol);
        std::shared_ptr<const data_type::SymbolDetail> query_symbol_detail(const data_type::Symbol& symbol) const;
        std::shared_ptr<const data_type::PositionData> query_position_data(const data_type::Symbol& symbol) const;
        const data_type::AccountData& query_account_data() const;
        const data_type::OrderData& query_order_data(data_type::OrderRef order_ref) const;
        const std::vector<data_type::TradeData>& query_trade_data(data_type::OrderRef order_ref) const;
        std::shared_ptr<const TradeInfo> query_trade_info() const;
        // trade
        std::optional<data_type::OrderRef> order_insert(uint32_t strategy_id, const data_type::OrderReq& req);
        bool order_cancel(uint32_t strategy_id, data_type::OrderRef order_ref);
        void algo_insert(const data_type::AlgoReq& req);


    private:
        void execute();
        bool init_trade_info();
        bool init_market_info();
        std::unordered_set<data_type::Symbol> init_strategy(uint32_t strategy_id);
        config_type::EngineConfig _config;
        bool _is_trading;
        std::shared_ptr<event::EventLoop> _event_loop;
        pqxx::connection _db_reader;
        db::Executor _db_writer;
        std::unique_ptr<std::jthread> _busy_worker;
        std::unique_ptr<gateway::Gateway> _gateway;
        std::shared_ptr<const TradeInfo> _trade_info;
        std::shared_ptr<const MarketInfo> _market_info;
        std::unordered_set<data_type::Symbol> _subscribed_symbols;
        std::shared_ptr<const RiskIndicators> _risk_indicators = std::make_shared<const RiskIndicators>();
        std::unique_ptr<OMS> _oms;
        std::unique_ptr<RiskControl> _risk_control;
        std::unique_ptr<TradingContext> _context;
        std::vector<std::shared_ptr<interface::Strategy>> _strategies;
        std::unordered_map<data_type::Symbol, std::tuple<std::string, std::shared_ptr<algo::Algo>>> _algos;
    };
};