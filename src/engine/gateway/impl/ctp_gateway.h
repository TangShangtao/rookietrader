#pragma once
#include <set>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <condition_variable>
#include <ThostFtdcMdApi.h>
#include <ThostFtdcTraderApi.h>
#include "data_type.h"
#include "gateway/gateway.h"

namespace rk::gateway
{

    class CTPGateway;
    class CTPTradeHandler final : public CThostFtdcTraderSpi
    {
    public:
        explicit CTPTradeHandler(CTPGateway* gateway):_gateway(gateway){}
        virtual ~CTPTradeHandler() = default;
        void OnFrontConnected() noexcept override;
        void OnFrontDisconnected(int nReason) noexcept override;
        void OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept override;

        void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRtnOrder(CThostFtdcOrderField* pOrder) noexcept override;
        void OnRtnTrade(CThostFtdcTradeField *pTrade) noexcept override;
        void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept override;

    private:
        CTPGateway* _gateway;
    };
    class CTPMarketHandler final : public CThostFtdcMdSpi
    {
    public:
        explicit CTPMarketHandler(CTPGateway* gateway):_gateway(gateway){}
        virtual ~CTPMarketHandler() = default;
        void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnFrontConnected() noexcept override;
        void OnFrontDisconnected(int nReason) noexcept override;
        void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept override;
        void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData) noexcept override;
    private:
        CTPGateway* _gateway;
    };
    struct CTPMDDeleter { void operator()(CThostFtdcMdApi* p){if (p) {p->Release();}}};
    struct CTPTDDeleter { void operator()(CThostFtdcTraderApi* p){if (p) {p->Release();}}};
    class CTPGateway final : public Gateway
    {
        friend class CTPMarketHandler;
        friend class CTPTradeHandler;
    public:
        CTPGateway(std::shared_ptr<event::EventLoop> event_loop, config_type::GatewayConfig config);
        // gateway
        bool login_trade() override;
        bool login_market() override;
        void logout_trade() override;
        void logout_market() override;
        bool subscribe(std::unordered_set<data_type::Symbol> symbols) override;
        uint32_t query_trading_day() override;
        std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>>> query_symbol_details() override;
        std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::PositionData>>> query_position_data() override;
        std::optional<std::vector<data_type::OrderData>> query_order_data() override;
        std::optional<std::vector<std::vector<data_type::TradeData>>> query_trade_data() override;
        std::optional<data_type::AccountData> query_account_data() override;
        void order_insert(data_type::OrderRef order_ref, const data_type::OrderReq& order_req) override;
        void order_cancel(data_type::OrderRef order_ref) override;
    private:
        // trade
        bool do_auth();
        bool do_trader_login();
        bool do_submit_settlement();
        // market
        bool do_market_login();
        bool do_subscribe(const std::unordered_set<data_type::Symbol>& symbols);
        // converters
        static data_type::ProductClass convert_product_class(TThostFtdcProductClassType counter_field);
        static data_type::Exchange convert_exchange(TThostFtdcExchangeIDType counter_field);
        static constexpr std::string_view convert_exchange(data_type::Exchange field);
        static constexpr TThostFtdcDirectionType convert_direction(data_type::Direction field);
        static constexpr TThostFtdcOffsetFlagType convert_offset(data_type::Offset field, data_type::Exchange exchange);
        static constexpr data_type::Offset convert_offset(TThostFtdcOffsetFlagType field);
        static util::DateTime convert_trading_day_to_natural_day(TThostFtdcDateType trading_day, TThostFtdcTimeType update_time, TThostFtdcMillisecType millisec);
        // members
        CTPTradeHandler _td_handler;
        CTPMarketHandler _md_handler;
        std::unique_ptr<CThostFtdcTraderApi, CTPTDDeleter> _td_api;
        std::unique_ptr<CThostFtdcMdApi, CTPMDDeleter> _md_api;
        std::mutex _gateway_status_mutex;
        std::condition_variable _gateway_status_cv;
        std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>> _symbol_details;
        std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::PositionData>> _position_data;
        std::vector<data_type::OrderData> _order_data;
        std::vector<std::vector<data_type::TradeData>> _trade_data;
        data_type::AccountData _account_data;
        int _req_id = 0;
        int _front_id = 0;
        int _session_id = 0;
        uint32_t _trading_day = 0;
        // 柜台额外需要字段
        bool _md_continue_login = true;
        bool _td_continue_login = true;
        std::vector<std::string> _order_ref_to_trade_symbol;    // FrontID + SessionID + OrderRef撤单需要InstrumentID字段
        std::unordered_map<std::string, data_type::Symbol> _trade_symbol_to_symbol;
        std::unordered_multimap<std::string, std::shared_ptr<data_type::SymbolDetail>> _underlying_to_symbol_details;
    };

};