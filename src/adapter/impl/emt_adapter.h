#pragma once
#include <emt_trader_api.h>
#include <quote_api.h>
#include "../adapter.h"
#include <condition_variable>
#include <mutex>

namespace rk::adapter
{

    struct EMTMDDeleter { void operator()(EMQ::API::QuoteApi* p){if (p) {;}} };
    struct EMTTDDeleter { void operator()(EMT::API::TraderApi* p){if (p) {p->Release();}} };
    class EMTTDAdapter final : public TDAdapter, public EMT::API::TraderSpi
    {
    public:
        EMTTDAdapter(TDAdapter::PushDataCallbacks push_data_callbacks, config_type::TDAdapterConfig config);
        ~EMTTDAdapter() override = default;
        bool login() override;
        void logout() override;
        uint32_t query_trading_day() override;
        std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::PositionData>>> query_position_data() override;
        std::optional<std::vector<data_type::OrderData>> query_order_data() override;
        std::optional<std::vector<std::vector<data_type::TradeData>>> query_trade_data() override;
        std::optional<data_type::AccountData> query_account_data() override;
        void order_insert(data_type::OrderRef order_ref, const data_type::OrderReq& order_req) override;
        void order_cancel(data_type::OrderRef order_ref) override;
        void notify_rpc_result(RPCResult result);
    private:
        void OnConnected() override;
        void OnDisconnected(int reason) override;
        void OnOrderEvent(EMTOrderInfo *order_info, EMTRI *error_info, uint64_t session_id) override;
        void OnTradeEvent(EMTTradeReport *trade_info, uint64_t session_id) override;
        void OnCancelOrderError(EMTOrderCancelInfo *cancel_info, EMTRI *error_info, uint64_t session_id) override;
        void OnQueryPositionByPage(EMTQueryStkPositionRsp *trade_info, int64_t req_count, int64_t trade_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id) override;
        void OnQueryOrderByPage(EMTQueryOrderRsp *order_info, int64_t req_count, int64_t order_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id) override;
        void OnQueryTradeByPage(EMTQueryTradeRsp *trade_info, int64_t req_count, int64_t trade_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id) override;
        void OnQueryAsset(EMTQueryAssetRsp *asset, EMTRI *error_info, int request_id, bool is_last, uint64_t session_id) override;
        std::unique_ptr<EMT::API::TraderApi, EMTTDDeleter> _td_api;
        RPCResult _rpc_result = RPCResult::UNKNOWN;
        std::mutex _rpc_mutex;
        std::condition_variable _rpc_condition_variable;
        std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::PositionData>> _position_data;
        std::vector<data_type::OrderData> _order_data;
        std::vector<std::vector<data_type::TradeData>> _trade_data;
        data_type::AccountData _account_data;
        int _req_id = 0;
        uint64_t _session_id = 0;
        uint32_t _trading_day = 0;
        // 柜台额外需要字段
        int _symbol_detail_query_count = 0;
        std::vector<uint64_t> _order_ref_to_emt_order_id;
        std::unordered_map<uint64_t, data_type::OrderRef> _emt_order_id_to_order_ref;
    };
    class EMTMDAdapter final : public MDAdapter, public EMQ::API::QuoteSpi, public EMT::API::TraderSpi
    {
    public:
        EMTMDAdapter(MDAdapter::PushDataCallbacks push_data_callbacks, config_type::MDAdapterConfig config);
        ~EMTMDAdapter() override = default;
        bool login() override;
        void logout() override;
        bool subscribe(std::unordered_set<data_type::Symbol> symbols) override;
        bool unsubscribe(std::unordered_set<data_type::Symbol>) override;
        std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>>> query_symbol_detail() override;
        std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::ETFDetail>>> query_etf_detail() override;
        void notify_rpc_result(RPCResult result);
    private:
        bool do_subscribe(const std::unordered_set<data_type::Symbol>& symbols);
        bool do_unsubscribe(const std::unordered_set<data_type::Symbol>& symbols);
        void OnQueryAllTickers(EMTQuoteStaticInfo* qsi, EMTRspInfoStruct* error_info, bool is_last) override;
        void OnQueryETF(EMTQueryETFBaseRsp *etf_info, EMTRI *error_info, int request_id, bool is_last, uint64_t session_id) override;
        void OnQueryETFBasket(EMTQueryETFComponentRsp *etf_component_info, EMTRI *error_info, int request_id, bool is_last, uint64_t session_id) override;
        void OnSubMarketData(EMTSpecificTickerStruct* ticker, EMTRspInfoStruct* error_info, bool is_last) override;
        void OnUnSubMarketData(EMTSpecificTickerStruct* ticker, EMTRspInfoStruct* error_info, bool is_last) override;
        void OnDepthMarketData(EMTMarketDataStruct* market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count) override;
        std::unique_ptr<EMQ::API::QuoteApi, EMTMDDeleter> _md_api;
        RPCResult _rpc_result = RPCResult::UNKNOWN;
        std::mutex _rpc_mutex;
        std::condition_variable _rpc_condition_variable;
        std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>> _symbol_detail;
        std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::ETFDetail>> _etf_detail;
        std::unordered_set<data_type::Symbol> _subscribed_symbols;

    };
    class EMTAdapter
    {
    public:
        static data_type::ProductClass convert_product_class(EMQ_TICKER_TYPE counter_field);
        static data_type::Exchange convert_exchange(EMQ_EXCHANGE_TYPE counter_field);
        static data_type::Exchange convert_exchange(EMT_MARKET_TYPE counter_field);
        static EMT_MARKET_TYPE convert_exchange(data_type::Exchange field);
        static EMQ_EXCHANGE_TYPE convert_exchange1(data_type::Exchange field);
        static EMT_SIDE_TYPE convert_direction(data_type::Direction field);
        static data_type::Direction convert_direction(EMT_SIDE_TYPE field);
        static EMT_POSITION_EFFECT_TYPE convert_offset(data_type::Offset field);
        static data_type::Offset convert_offset(EMT_POSITION_EFFECT_TYPE field);
        static EMT_BUSINESS_TYPE_EXT convert_business_type(data_type::Direction field);
        static std::string convert_trade_symbol_to_symbol(data_type::Exchange exchange, std::string trade_symbol);
        static util::DateTime convert_datetime(int64_t time);
    };

};