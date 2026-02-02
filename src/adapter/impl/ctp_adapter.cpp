#include "ctp_adapter.h"
#include "util/logger.h"
#include "util/str.h"
#include <magic_enum/magic_enum.hpp>
namespace rk::adapter
{
    void CTPTradeHandler::OnFrontConnected() noexcept
    {
        RK_LOG_INFO("trade front connected");
        if (!_td_gateway->_continue_login) return;
        if (!_td_gateway->do_auth())
        {
            _td_gateway->notify_rpc_result(RPCResult::FAILED);
        }
    }
    void CTPTradeHandler::OnFrontDisconnected(int nReason) noexcept
    {
        RK_LOG_INFO("trade front disconnected. reason {}", nReason);
        _td_gateway->_continue_login = false;
        _td_gateway->_push_data_callbacks.push_td_disconnected();
    }
    void CTPTradeHandler::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            _td_gateway->notify_rpc_result(RPCResult::FAILED);;
            return;
        }
        if (!_td_gateway->do_trader_login())
        {
            _td_gateway->notify_rpc_result(RPCResult::FAILED);
            return;
        }
    }
    void CTPTradeHandler::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            _td_gateway->notify_rpc_result(RPCResult::FAILED);;
            return;
        }
        _td_gateway->_front_id = pRspUserLogin->FrontID;
        _td_gateway->_session_id = pRspUserLogin->SessionID;
        if (!_td_gateway->do_submit_settlement())
        {
            _td_gateway->notify_rpc_result(RPCResult::FAILED);;
        }
    }
    void CTPTradeHandler::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {

    }
    void CTPTradeHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            _td_gateway->notify_rpc_result(RPCResult::FAILED);;
            return;
        }
        _td_gateway->notify_rpc_result(RPCResult::SUCCESS);
    }
    void CTPTradeHandler::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            auto error_msg = util::gbk2utf8(pRspInfo->ErrorMsg);
            RK_LOG_WARN("order ref {} order insert failed, res {}, msg {}", pInputOrder->OrderRef, pRspInfo->ErrorID, error_msg);
            _td_gateway->_push_data_callbacks.push_order_error({
                _td_gateway->_trading_day,
                static_cast<uint32_t>(std::stoi(pInputOrder->OrderRef)),
                data_type::ErrorType::ORDER_INSERT_ERROR,
                std::format("ctp error id {}, error msg {}", pRspInfo->ErrorID, error_msg)
            });
        }
    }
    void CTPTradeHandler::OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            auto error_msg = util::gbk2utf8(pRspInfo->ErrorMsg);
            RK_LOG_WARN("order ref {} cancel failed, res {}, msg {}", pInputOrderAction->OrderRef, pRspInfo->ErrorID, error_msg);
            _td_gateway->_push_data_callbacks.push_order_error({
                _td_gateway->_trading_day,
                static_cast<uint32_t>(std::stoi(pInputOrderAction->OrderRef)),
                data_type::ErrorType::ORDER_CANCEL_ERROR,
                std::format("ctp error id {}, error msg {}", pRspInfo->ErrorID, error_msg)
            });
        }
    }
    void CTPTradeHandler::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            _td_gateway->notify_rpc_result(RPCResult::FAILED);
            return;
        }
        if (!pInvestorPosition)
        {
            RK_LOG_WARN("{} data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto symbol = data_type::Symbol{pInvestorPosition->InstrumentID};
            auto [it, _] = _td_gateway->_position_data.try_emplace(symbol, std::make_shared<data_type::PositionData>(symbol));
            switch (pInvestorPosition->PosiDirection)
            {
                case THOST_FTDC_PD_Long:
                {
                    it->second->long_position.position += pInvestorPosition->Position;
                    break;
                }
                case THOST_FTDC_PD_Short:
                {
                    it->second->short_position.position += pInvestorPosition->Position;
                    break;
                }
                default: RK_LOG_ERROR("unknown position direction {}", pInvestorPosition->PosiDirection);
            }
        }
        if (bIsLast)
        {
            _td_gateway->notify_rpc_result(RPCResult::SUCCESS);
        }
    }
    void CTPTradeHandler::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            _td_gateway->notify_rpc_result(RPCResult::FAILED);
            return;
        }
        if (!pOrder)
        {
            RK_LOG_WARN("{} data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto order_ref = static_cast<data_type::OrderRef>(std::stoul(pOrder->OrderRef));
            if (order_ref >= _td_gateway->_order_data.size())
            {
                _td_gateway->_order_data.resize(order_ref + 1);
            }
            _td_gateway->_order_data[order_ref] = data_type::OrderData{
                order_ref,
                {
                    {pOrder->InstrumentID},
                    pOrder->LimitPrice,
                    static_cast<uint32_t>(pOrder->VolumeTotal),
                    pOrder->Direction == THOST_FTDC_D_Buy ? data_type::Direction::LONG : data_type::Direction::SHORT,
                    CTPAdapter::convert_offset(pOrder->CombOffsetFlag[0]),
                },
                _td_gateway->_trading_day,
                util::DateTime::strptime(
                    std::format("{} {}", pOrder->InsertDate, pOrder->InsertTime),
                    "%Y%m%d %H:%M:%S"
                ),
                static_cast<uint32_t>(pOrder->VolumeTraded),
                static_cast<uint32_t>(std::strlen(pOrder->CancelTime) != 0 ? 0 : (pOrder->VolumeTotal -
                                                                                  pOrder->VolumeTraded)),
                static_cast<uint32_t>(std::strlen(pOrder->CancelTime) == 0 ? 0 : (pOrder->VolumeTotal -
                                                                                  pOrder->VolumeTraded))
            };

        }
        if (bIsLast)
        {
            _td_gateway->notify_rpc_result(RPCResult::SUCCESS);
        }
    }
    void CTPTradeHandler::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)  noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            _td_gateway->notify_rpc_result(RPCResult::FAILED);
            return;
        }
        if (!pTrade)
        {
            RK_LOG_WARN("{} data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto order_ref = static_cast<data_type::OrderRef>(std::stoul(pTrade->OrderRef));
            if (order_ref >= _td_gateway->_trade_data.size())
            {
                _td_gateway->_trade_data.resize(order_ref + 1);
            }
            _td_gateway->_trade_data[order_ref].emplace_back(data_type::TradeData{
                order_ref,
                pTrade->TradeID,
                pTrade->Price,
                static_cast<uint32_t>(pTrade->Volume),
                _td_gateway->_trading_day,
                util::DateTime::strptime(
                    std::format("{} {}", pTrade->TradeDate, pTrade->TradeTime),
                    "%Y%m%d %H:%M:%S"
                ),
                0
            });
        }
        if (bIsLast)
        {
            _td_gateway->notify_rpc_result(RPCResult::SUCCESS);
        }
    }
    void CTPTradeHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            _td_gateway->notify_rpc_result(RPCResult::FAILED);
            return;
        }
        if (!pTradingAccount)
        {
            RK_LOG_WARN("{} data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            _td_gateway->_account_data.trading_day = _td_gateway->_trading_day;
            _td_gateway->_account_data.update_time = util::DateTime::now();
            _td_gateway->_account_data.balance = pTradingAccount->Balance;
            _td_gateway->_account_data.pre_balance = pTradingAccount->PreBalance;
            _td_gateway->_account_data.deposit = pTradingAccount->Deposit;
            _td_gateway->_account_data.withdraw = pTradingAccount->Withdraw;
            _td_gateway->_account_data.margin = pTradingAccount->CurrMargin;
            _td_gateway->_account_data.pre_margin = pTradingAccount->PreMargin;
            _td_gateway->_account_data.frozen_margin = pTradingAccount->FrozenMargin;
            _td_gateway->_account_data.commission = pTradingAccount->Commission;
            _td_gateway->_account_data.available = pTradingAccount->Available;

        }
        if (bIsLast)
        {
            _td_gateway->notify_rpc_result(RPCResult::SUCCESS);
        }
    }
    void CTPTradeHandler::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
        }
    }
    void CTPTradeHandler::OnRtnOrder(CThostFtdcOrderField* pOrder) noexcept
    {
        if (!pOrder) return;
        if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled && std::strlen(pOrder->CancelTime) == 0)
        {
            auto error_msg = util::gbk2utf8(pOrder->StatusMsg);
            RK_LOG_WARN("order ref {} order insert rejected, reason {}", pOrder->OrderRef, error_msg.c_str());
            _td_gateway->_push_data_callbacks.push_order_error({
                _td_gateway->_trading_day,
                static_cast<uint32_t>(std::stoi(pOrder->OrderRef)),
                data_type::ErrorType::ORDER_INSERT_ERROR,
                std::format("ctp order insert rejected, reason: {}", std::move(error_msg))
            });
            return;
        }
        if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
        {
            auto cancel_time = util::DateTime::strptime(std::format("{} {}", pOrder->TradingDay, pOrder->CancelTime),"%Y%m%d %H:%M:%S",0, 0, 0);
            _td_gateway->_push_data_callbacks.push_cancel({
               static_cast<data_type::OrderRef>(std::stoul(pOrder->OrderRef)),
               static_cast<uint32_t>(pOrder->VolumeTotal - pOrder->VolumeTraded),
               static_cast<uint32_t>(std::stoi(pOrder->TradingDay)),
               cancel_time,
           });
            return;
        }

    }
    void CTPTradeHandler::OnRtnTrade(CThostFtdcTradeField *pTrade) noexcept
    {
        if (!pTrade) return;
        _td_gateway->_push_data_callbacks.push_trade({
            static_cast<data_type::OrderRef>(std::stoul(pTrade->OrderRef)),
            pTrade->TradeID,
            pTrade->Price,
            static_cast<uint32_t>(pTrade->Volume),
            static_cast<uint32_t>(std::stoi(pTrade->TradingDay)),
            util::DateTime::strptime(std::format("{} {}", pTrade->TradeDate, pTrade->TradeTime),"%Y%m%d %H:%M:%S",0, 0, 0),
            0
        });
    }

    CTPTDAdapter::CTPTDAdapter(TDAdapter::PushDataCallbacks push_data_callbacks, config_type::TDAdapterConfig config)
    : TDAdapter(std::move(push_data_callbacks), std::move(config)),
      _handler(this),
      _order_ref_to_trade_symbol(1000)
    {
        
    }
    bool CTPTDAdapter::login()
    {
        const auto trade_flow_path = std::format("ctp_trade_flow/{}/", _config.user_id);
        if (!std::filesystem::exists(trade_flow_path))
        {
            std::filesystem::create_directories(trade_flow_path);
        }
        _td_api = std::unique_ptr<CThostFtdcTraderApi, CTPTDDeleter>(
            CThostFtdcTraderApi::CreateFtdcTraderApi(trade_flow_path.c_str()),
            CTPTDDeleter()
        );
        _td_api->RegisterSpi(&_handler);
        _td_api->RegisterFront(const_cast<char*>(std::format("{}://{}:{}", _config.sock_type, _config.trade_front_ip, _config.trade_front_port).c_str()));
        _td_api->SubscribePrivateTopic(THOST_TE_RESUME_TYPE::THOST_TERT_QUICK);
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        _continue_login = true;
        _td_api->Init();
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return _rpc_result != RPCResult::UNKNOWN;
                }
            )
        )
        {
            RK_LOG_ERROR("login trade front time out");
            return false;
        }
        if (_rpc_result == RPCResult::FAILED)
        {
            RK_LOG_ERROR("login trade front error");
            return false;
        }
        _rpc_result = RPCResult::UNKNOWN;
        return true;
    }
    void CTPTDAdapter::logout()
    {
        CThostFtdcUserLogoutField field{};
        std::strncpy(field.BrokerID, _config.broker_id.c_str(), sizeof(field.BrokerID) - 1);
        std::strncpy(field.UserID, _config.user_id.c_str(), sizeof(field.UserID) - 1);
        _td_api->ReqUserLogout(&field, ++_req_id);
        _td_api = nullptr;
        _rpc_result = RPCResult::UNKNOWN;
    }
    uint32_t CTPTDAdapter::query_trading_day()
    {
        _trading_day = std::stol(_td_api->GetTradingDay());
        return _trading_day;
    }
    std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::PositionData>>> CTPTDAdapter::query_position_data()
    {
        _position_data.clear();
        CThostFtdcQryInvestorPositionField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID) - 1);
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        auto res = _td_api->ReqQryInvestorPosition(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("query position return {}", res);
            return std::nullopt;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return _rpc_result != RPCResult::UNKNOWN;
                }
            )
        )
        {
            RK_LOG_ERROR("query position data time out");
            return std::nullopt;
        }
        if (_rpc_result == RPCResult::FAILED)
        {
            RK_LOG_ERROR("query position data error");
            return std::nullopt;
        }
        _rpc_result = RPCResult::UNKNOWN;
        return std::move(_position_data);
    }
    std::optional<std::vector<data_type::OrderData>> CTPTDAdapter::query_order_data()
    {
        _order_data.clear();
        CThostFtdcQryOrderField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID) - 1);
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        auto res = _td_api->ReqQryOrder(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("query order return {}", res);
            return std::nullopt;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return _rpc_result != RPCResult::UNKNOWN;
                }
            )
        )
        {
            RK_LOG_ERROR("query order data time out");
            return std::nullopt;
        }
        if (_rpc_result == RPCResult::FAILED)
        {
            RK_LOG_ERROR("query order data error");
            return std::nullopt;
        }
        _rpc_result = RPCResult::UNKNOWN;
        return std::move(_order_data);
    }
    std::optional<std::vector<std::vector<data_type::TradeData>>> CTPTDAdapter::query_trade_data()
    {
        RK_LOG_INFO("start query trade data...");
        _trade_data.clear();
        CThostFtdcQryTradeField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID) - 1);
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        auto res = _td_api->ReqQryTrade(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("query trade return {}", res);
            return std::nullopt;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return _rpc_result != RPCResult::UNKNOWN;
                }
            )
        )
        {
            RK_LOG_ERROR("query trade data time out");
            return std::nullopt;
        }
        if (_rpc_result == RPCResult::FAILED)
        {
            RK_LOG_ERROR("query trade data error");
            return std::nullopt;
        }
        _rpc_result = RPCResult::UNKNOWN;
        return std::move(_trade_data);
    }
    std::optional<data_type::AccountData> CTPTDAdapter::query_account_data()
    {
        RK_LOG_INFO("start query account data...");
        CThostFtdcQryTradingAccountField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID) - 1);
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        auto res = _td_api->ReqQryTradingAccount(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("query account return {}", res);
            return std::nullopt;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return _rpc_result != RPCResult::UNKNOWN;
                }
            )
        )
        {
            RK_LOG_ERROR("query account data time out");
            return std::nullopt;
        }
        if (_rpc_result == RPCResult::FAILED)
        {
            RK_LOG_ERROR("query account data error");
            return std::nullopt;
        }
        _rpc_result = RPCResult::UNKNOWN;
        return std::move(_account_data);
    }
    void CTPTDAdapter::order_insert(data_type::OrderRef order_ref, const data_type::OrderReq& order_req)
    {
        CThostFtdcInputOrderField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID) - 1);
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID) - 1);
        std::sprintf(req.OrderRef, "%u", order_ref);
        req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
        req.Direction = CTPAdapter::convert_direction(order_req.direction);
        req.CombOffsetFlag[0] = CTPAdapter::convert_offset(order_req.offset, order_req.symbol.exchange);
        req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
        req.LimitPrice = order_req.limit_price;
        req.VolumeTotalOriginal = static_cast<int>(order_req.volume);
        req.TimeCondition = THOST_FTDC_TC_GFD;
        req.VolumeCondition = THOST_FTDC_VC_AV;
        req.MinVolume = 1;
        req.ContingentCondition = THOST_FTDC_CC_Immediately;
        req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
        req.IsAutoSuspend = 0;
        _order_ref_to_trade_symbol[order_ref] = order_req.symbol.trade_symbol;
        std::strncpy(req.ExchangeID, CTPAdapter::convert_exchange(order_req.symbol.exchange).data(), sizeof(req.ExchangeID));
        std::strncpy(req.InstrumentID, order_req.symbol.trade_symbol.c_str(), sizeof(req.InstrumentID));
        req.UserForceClose = 0;
        auto res = _td_api->ReqOrderInsert(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("order insert return {}", res);
            _push_data_callbacks.push_order_error({
                _trading_day,
                order_ref,
                data_type::ErrorType::ORDER_INSERT_ERROR,
                std::format("api return {}", res)
            });
        }
    }
    void CTPTDAdapter::order_cancel(data_type::OrderRef order_ref)
    {
        CThostFtdcInputOrderActionField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID) - 1);
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID) - 1);
        std::strncpy(req.InstrumentID, _order_ref_to_trade_symbol[order_ref].c_str(), sizeof(req.InstrumentID));
        std::sprintf(req.OrderRef, "%u", order_ref);
        req.FrontID = _front_id;
        req.SessionID = _session_id;
        req.ActionFlag = THOST_FTDC_AF_Delete;
        auto res = _td_api->ReqOrderAction(&req, ++_req_id);
        if (res)
        {
            RK_LOG_ERROR("order cancel return {}", res);
            _push_data_callbacks.push_order_error({
                _trading_day,
                order_ref,
                data_type::ErrorType::ORDER_CANCEL_ERROR,
                std::format("api return {}", res)
            });
        }
    }
    bool CTPTDAdapter::do_auth()
    {
        CThostFtdcReqAuthenticateField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID) - 1);
        std::strncpy(req.AuthCode, _config.auth_code.c_str(), sizeof(req.AuthCode) - 1);
        std::strncpy(req.AppID, _config.app_id.c_str(), sizeof(req.AppID) - 1);
        auto res = _td_api->ReqAuthenticate(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("req auth return {}", res);
            return false;
        }
        return true ;
    }
    bool CTPTDAdapter::do_trader_login()
    {
        CThostFtdcReqUserLoginField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID) - 1);
        std::strncpy(req.Password, _config.password.c_str(), sizeof(req.Password) - 1);
        auto res = _td_api->ReqUserLogin(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("login trade front return {}", res);
            return false;
        }
        return true;
    }
    bool CTPTDAdapter::do_submit_settlement()
    {
        CThostFtdcSettlementInfoConfirmField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID) - 1);

        auto res = _td_api->ReqSettlementInfoConfirm(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("req settlement confirm return {}", res);
            return false;
        }
        return true;
    }
    void CTPTDAdapter::notify_rpc_result(RPCResult result)
    {
        {
            auto lock = std::unique_lock(_rpc_mutex);
            _rpc_result = result;
        }
        _rpc_condition_variable.notify_one();
    }
    void CTPMarketHandler::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
        }
    }
    void CTPMarketHandler::OnFrontConnected() noexcept
    {
        RK_LOG_INFO("market front connected");
        if (!_gateway->_continue_login) return;
        if (!_gateway->do_market_login())
        {
            _gateway->notify_rpc_result(RPCResult::FAILED);
        }
    }
    void CTPMarketHandler::OnFrontDisconnected(int nReason) noexcept
    {
        RK_LOG_WARN("market front disconnected. reason {}", nReason);
        _gateway->_continue_login = false;
        _gateway->_push_data_callbacks.push_md_disconnected();

    }
    void CTPMarketHandler::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            _gateway->notify_rpc_result(RPCResult::FAILED);
            return;
        }
        _gateway->notify_rpc_result(RPCResult::SUCCESS);
    }
    void CTPMarketHandler::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {

    }
    void CTPMarketHandler::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            _gateway->notify_rpc_result(RPCResult::FAILED);
            return;
        }
        if (bIsLast)
        {
            _gateway->notify_rpc_result(RPCResult::SUCCESS);
        }
    }
    void CTPMarketHandler::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            _gateway->notify_rpc_result(RPCResult::FAILED);
            return;
        }
        if (bIsLast)
        {
            _gateway->notify_rpc_result(RPCResult::SUCCESS);
        }
    }

    void CTPMarketHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData) noexcept
    {
        if (!pDepthMarketData) return;
        _gateway->_push_data_callbacks.push_tick({
            _gateway->_symbol_detail.find({pDepthMarketData->InstrumentID})->first,
            static_cast<uint32_t>(std::stoul(pDepthMarketData->TradingDay)),
            CTPAdapter::convert_trading_day_to_natural_day(pDepthMarketData->TradingDay, pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec),
            pDepthMarketData->LastPrice,
            pDepthMarketData->OpenPrice,
            pDepthMarketData->HighestPrice,
            pDepthMarketData->LowestPrice,
            pDepthMarketData->UpperLimitPrice,
            pDepthMarketData->LowerLimitPrice,
            static_cast<int64_t>(pDepthMarketData->Volume),
            pDepthMarketData->OpenInterest,
            pDepthMarketData->AveragePrice,
            {
                pDepthMarketData->BidPrice1,
                pDepthMarketData->BidPrice2,
                pDepthMarketData->BidPrice3,
                pDepthMarketData->BidPrice4,
                pDepthMarketData->BidPrice5
            },
            {
                pDepthMarketData->AskPrice1,
                pDepthMarketData->AskPrice2,
                pDepthMarketData->AskPrice3,
                pDepthMarketData->AskPrice4,
                pDepthMarketData->AskPrice5
            },
            {
                static_cast<int64_t>(pDepthMarketData->BidVolume1),
                static_cast<int64_t>(pDepthMarketData->BidVolume2),
                static_cast<int64_t>(pDepthMarketData->BidVolume3),
                static_cast<int64_t>(pDepthMarketData->BidVolume4),
                static_cast<int64_t>(pDepthMarketData->BidVolume5)
            },
            {
                static_cast<int64_t>(pDepthMarketData->AskVolume1),
                static_cast<int64_t>(pDepthMarketData->AskVolume2),
                static_cast<int64_t>(pDepthMarketData->AskVolume3),
                static_cast<int64_t>(pDepthMarketData->AskVolume4),
                static_cast<int64_t>(pDepthMarketData->AskVolume5)
            },

        });
    }
    CTPMDAdapter::CTPMDAdapter(PushDataCallbacks push_data_callbacks, config_type::MDAdapterConfig config)
        :
        MDAdapter(std::move(push_data_callbacks), std::move(config)),
        _md_handler(this)
    {

    }
    bool CTPMDAdapter::login()
    {
        const auto market_flow_path = std::format("ctp_market_flow/{}/", _config.user_id);
        if (!std::filesystem::exists(market_flow_path))
        {
            std::filesystem::create_directories(market_flow_path);
        }
        _md_api = std::unique_ptr<CThostFtdcMdApi, CTPMDDeleter>(
            CThostFtdcMdApi::CreateFtdcMdApi(market_flow_path.c_str()),
            CTPMDDeleter()
        );
        _md_api->RegisterSpi(&_md_handler);
        _md_api->RegisterFront(const_cast<char*>(std::format("{}://{}:{}", _config.sock_type, _config.market_front_ip, _config.market_front_port).c_str()));
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        _continue_login = true;
        _md_api->Init();
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return _rpc_result != RPCResult::UNKNOWN;
                }
            )
        )
        {
            RK_LOG_ERROR("login market front time out");
            return false;
        }
        if (_rpc_result == RPCResult::FAILED)
        {
            RK_LOG_ERROR("login market front error");
            return false;
        }
        _rpc_result = RPCResult::UNKNOWN;
        return true;
    }
    void CTPMDAdapter::logout()
    {
        _md_api = nullptr;
        _rpc_result = RPCResult::UNKNOWN;
    }
    bool CTPMDAdapter::subscribe(std::unordered_set<data_type::Symbol> symbols)
    {
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        if (!do_subscribe(symbols))
        {
            RK_LOG_ERROR("subscribe error");
            return false;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return _rpc_result != RPCResult::UNKNOWN;
                }
            )
        )
        {
            RK_LOG_ERROR("subscribe time out");
            return false;
        }
        if (_rpc_result == RPCResult::FAILED)
        {
            RK_LOG_ERROR("subscribe error");
            return false;
        }
        _rpc_result = RPCResult::UNKNOWN;
        return true;
    }
    bool CTPMDAdapter::unsubscribe(std::unordered_set<data_type::Symbol> symbols)
    {
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        if (!do_unsubscribe(symbols))
        {
            RK_LOG_ERROR("unsubscribe error");
            return false;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return _rpc_result != RPCResult::UNKNOWN;
                }
            )
        )
        {
            RK_LOG_ERROR("unsubscribe time out");
            return false;
        }
        if (_rpc_result == RPCResult::FAILED)
        {
            RK_LOG_ERROR("unsubscribe error");
            return false;
        }
        _rpc_result = RPCResult::UNKNOWN;
        return true;
    }
    std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>>> CTPMDAdapter::query_symbol_detail()
    {
        const auto trade_flow_path = std::format("ctp_trade_flow/{}/", _config.user_id);
        if (!std::filesystem::exists(trade_flow_path))
        {
            std::filesystem::create_directories(trade_flow_path);
        }
        _td_api = std::unique_ptr<CThostFtdcTraderApi, CTPTDDeleter>(
            CThostFtdcTraderApi::CreateFtdcTraderApi(trade_flow_path.c_str()),
            CTPTDDeleter()
        );
        _td_api->RegisterSpi(this);
        _td_api->RegisterFront(const_cast<char*>(std::format("{}://{}:{}", _config.sock_type, _config.trade_front_ip, _config.trade_front_port).c_str()));
        _td_api->SubscribePrivateTopic(THOST_TE_RESUME_TYPE::THOST_TERT_QUICK);
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        _td_api->Init();
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return _rpc_result != RPCResult::UNKNOWN;
                }
            )
        )
        {
            RK_LOG_ERROR("login trade front time out");
            return std::nullopt;
        }
        if (_rpc_result == RPCResult::FAILED)
        {
            RK_LOG_ERROR("login trade front error");
            return std::nullopt;
        }
        _rpc_result = RPCResult::UNKNOWN;
        _symbol_detail.clear();

        CThostFtdcQryInstrumentField req{};
        auto res = _td_api->ReqQryInstrument(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("query symbol req return {}", res);
            return std::nullopt;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return _rpc_result != RPCResult::UNKNOWN;
                }
            )
        )
        {
            RK_LOG_ERROR("query symbol detail time out");
            return std::nullopt;
        }
        if (_rpc_result == RPCResult::FAILED)
        {
            RK_LOG_ERROR("query symbol detail error");
            return std::nullopt;
        }
        _rpc_result = RPCResult::UNKNOWN;
        CThostFtdcUserLogoutField field{};
        std::strncpy(field.BrokerID, _config.broker_id.c_str(), sizeof(field.BrokerID) - 1);
        std::strncpy(field.UserID, _config.user_id.c_str(), sizeof(field.UserID) - 1);
        _td_api->ReqUserLogout(&field, ++_req_id);
        _td_api = nullptr;
        return _symbol_detail;

    }
    void CTPMDAdapter::notify_rpc_result(RPCResult result)
    {
        {
            auto lock = std::unique_lock(_rpc_mutex);
            _rpc_result = result;
        }
        _rpc_condition_variable.notify_one();
    }
    void CTPMDAdapter::OnFrontConnected() noexcept
    {
        RK_LOG_INFO("trade front connected");
        if (!do_auth())
        {
            notify_rpc_result(RPCResult::FAILED);
        }
    }
    void CTPMDAdapter::OnFrontDisconnected(int nReason) noexcept
    {
        RK_LOG_INFO("trade front disconnected. reason {}", nReason);
    }
    void CTPMDAdapter::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            notify_rpc_result(RPCResult::FAILED);;
            return;
        }
        if (!do_trader_login())
        {
            notify_rpc_result(RPCResult::FAILED);
            return;
        }
    }
    void CTPMDAdapter::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            notify_rpc_result(RPCResult::FAILED);;
            return;
        }
        if (!do_submit_settlement())
        {
            notify_rpc_result(RPCResult::FAILED);;
        }
    }
    void CTPMDAdapter::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        
    }
    void CTPMDAdapter::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            notify_rpc_result(RPCResult::FAILED);;
            return;
        }
        notify_rpc_result(RPCResult::SUCCESS);
    }
    void CTPMDAdapter::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            notify_rpc_result(RPCResult::FAILED);
            return;
        }
        if (!pInstrument)
        {
            RK_LOG_WARN("{} data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto product_class = CTPAdapter::convert_product_class(pInstrument->ProductClass);
            if (
                std::find(
                    _config.product_class.begin(),
                    _config.product_class.end(),
                    magic_enum::enum_name(product_class)
                ) != _config.product_class.end()
                )
            {
                auto symbol = data_type::Symbol{
                    pInstrument->InstrumentID,
                    pInstrument->InstrumentID,
                    CTPAdapter::convert_exchange(pInstrument->ExchangeID),
                    product_class
                };
                auto symbol_detail = std::make_shared<data_type::SymbolDetail>(
                    symbol,
                    product_class,
                    pInstrument->UnderlyingInstrID,
                    util::gbk2utf8(pInstrument->InstrumentName),
                    pInstrument->PriceTick,
                    pInstrument->VolumeMultiple,
                    static_cast<uint32_t>(pInstrument->MinLimitOrderVolume),
                    static_cast<uint32_t>(pInstrument->MinLimitOrderVolume),
                    static_cast<uint32_t>(pInstrument->MaxLimitOrderVolume),
                    static_cast<uint32_t>(pInstrument->MaxLimitOrderVolume),
                    std::numeric_limits<double>::max(), // 涨停价
                    0.,                                 // 跌停价
                    static_cast<uint32_t>(std::stoul(pInstrument->OpenDate)),
                    static_cast<uint32_t>(std::stoul(pInstrument->ExpireDate)),
                    pInstrument->LongMarginRatio,
                    pInstrument->ShortMarginRatio,
                    0.,0.,
                    0.,0.,
                    0.,0.
                );
                _underlying_to_symbol_details.emplace(pInstrument->UnderlyingInstrID, symbol_detail);
                _symbol_detail.emplace(symbol, symbol_detail);
            }

        }
        if (bIsLast)
        {
            CThostFtdcQryInstrumentCommissionRateField req{};
            std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
            std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID) - 1);
            auto res = _td_api->ReqQryInstrumentCommissionRate(&req, ++_req_id);
            if (res != 0)
            {
                RK_LOG_ERROR("query symbol detail return {}", res);
                notify_rpc_result(RPCResult::FAILED);
            }
        }

    }
    void CTPMDAdapter::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            RK_LOG_ERROR("{} failed. error id: {}, error msg: {}", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg));
            notify_rpc_result(RPCResult::FAILED);
            return;
        }
        if (!pInstrumentCommissionRate)
        {
            RK_LOG_WARN("{} data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            // 手续费设置到品种级别
            if (
                _underlying_to_symbol_details.find(pInstrumentCommissionRate->InstrumentID) !=
                _underlying_to_symbol_details.end()
                )
            {
                auto range = _underlying_to_symbol_details.equal_range(pInstrumentCommissionRate->InstrumentID);
                for (auto it = range.first; it != range.second; ++it)
                {
                    it->second->open_fee_rate_by_money = pInstrumentCommissionRate->OpenRatioByMoney;
                    it->second->open_fee_rate_by_volume = pInstrumentCommissionRate->OpenRatioByVolume;
                    it->second->close_fee_rate_by_money = pInstrumentCommissionRate->CloseRatioByMoney;
                    it->second->close_fee_rate_by_volume = pInstrumentCommissionRate->CloseRatioByVolume;
                    it->second->close_today_fee_rate_by_money = pInstrumentCommissionRate->CloseTodayRatioByMoney;
                    it->second->close_today_fee_rate_by_volume = pInstrumentCommissionRate->CloseTodayRatioByVolume;
                }
            }
                // 手续费设置到合约级别
            else if (
                auto it = _symbol_detail.find({pInstrumentCommissionRate->InstrumentID});
                it != _symbol_detail.end()
            )
            {
                it->second->open_fee_rate_by_money = pInstrumentCommissionRate->OpenRatioByMoney;
                it->second->open_fee_rate_by_volume = pInstrumentCommissionRate->OpenRatioByVolume;
                it->second->close_fee_rate_by_money = pInstrumentCommissionRate->CloseRatioByMoney;
                it->second->close_fee_rate_by_volume = pInstrumentCommissionRate->CloseRatioByVolume;
                it->second->close_today_fee_rate_by_money = pInstrumentCommissionRate->CloseTodayRatioByMoney;
                it->second->close_today_fee_rate_by_volume = pInstrumentCommissionRate->CloseTodayRatioByVolume;
            }
            else
            {
                RK_LOG_WARN("{} {} symbol detail not found in map", pInstrumentCommissionRate->InstrumentID, pInstrumentCommissionRate->ExchangeID);
            }
        }
        if (bIsLast)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // 防止查询超过柜台流控
            notify_rpc_result(RPCResult::SUCCESS);
        }
    }


    bool CTPMDAdapter::do_market_login()
    {
        CThostFtdcReqUserLoginField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID) - 1);
        std::strncpy(req.Password, _config.password.c_str(), sizeof(req.Password) - 1);
        auto res = _md_api->ReqUserLogin(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("login market front return {}", res);
            return false;
        }
        return true;
    }
    bool CTPMDAdapter::do_subscribe(const std::unordered_set<data_type::Symbol>& symbols)
    {
        char** symbol_list = static_cast<char**>(std::malloc(symbols.size() * sizeof(char*)));

        int num = 0;
        for (const auto& it : symbols)
        {
            symbol_list[num] = const_cast<char*>(it.trade_symbol.c_str());
            num++;
        }
        auto res = _md_api->SubscribeMarketData(symbol_list, num);
        std::free(symbol_list);
        if (res != 0)
        {
            RK_LOG_ERROR("subscribe return {}", res);
            return false;
        }
        return true;
    }
    bool CTPMDAdapter::do_unsubscribe(const std::unordered_set<data_type::Symbol>& symbols)
    {
        char** symbol_list = static_cast<char**>(std::malloc(symbols.size() * sizeof(char*)));

        int num = 0;
        for (const auto& it : symbols)
        {
            symbol_list[num] = const_cast<char*>(it.trade_symbol.c_str());
            num++;
        }
        auto res = _md_api->UnSubscribeMarketData(symbol_list, num);
        std::free(symbol_list);
        if (res != 0)
        {
            RK_LOG_ERROR("unsubscribe return {}", res);
            return false;
        }
        return true;
    }
    bool CTPMDAdapter::do_auth()
    {
        CThostFtdcReqAuthenticateField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID) - 1);
        std::strncpy(req.AuthCode, _config.auth_code.c_str(), sizeof(req.AuthCode) - 1);
        std::strncpy(req.AppID, _config.app_id.c_str(), sizeof(req.AppID) - 1);
        auto res = _td_api->ReqAuthenticate(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("req auth return {}", res);
            return false;
        }
        return true ;
    }
    bool CTPMDAdapter::do_trader_login()
    {
        CThostFtdcReqUserLoginField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID) - 1);
        std::strncpy(req.Password, _config.password.c_str(), sizeof(req.Password) - 1);
        auto res = _td_api->ReqUserLogin(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("login trade front return {}", res);
            return false;
        }
        return true;
    }
    bool CTPMDAdapter::do_submit_settlement()
    {
        CThostFtdcSettlementInfoConfirmField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID) - 1);
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID) - 1);

        auto res = _td_api->ReqSettlementInfoConfirm(&req, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("req settlement confirm return {}", res);
            return false;
        }
        return true;
    }
    data_type::ProductClass CTPAdapter::convert_product_class(TThostFtdcProductClassType counter_field)
    {
        switch (counter_field)
        {
            case THOST_FTDC_PC_Futures: return data_type::ProductClass::FUTURE;
            default: return data_type::ProductClass::UNKNOWN;
        }
    }
    data_type::Exchange CTPAdapter::convert_exchange(TThostFtdcExchangeIDType counter_field)
    {
        if (std::strcmp(counter_field, "SHFE") == 0) return data_type::Exchange::SHFE;
        if (std::strcmp(counter_field, "CFFEX") == 0) return data_type::Exchange::CFFEX;
        if (std::strcmp(counter_field, "INE") == 0) return data_type::Exchange::INE;
        if (std::strcmp(counter_field, "DCE") == 0) return data_type::Exchange::DCE;
        if (std::strcmp(counter_field, "CZCE") == 0) return data_type::Exchange::CZCE;
        if (std::strcmp(counter_field, "GFEX") == 0) return data_type::Exchange::GFEX;
        return data_type::Exchange::UNKNOWN;
    }
    constexpr std::string_view CTPAdapter::convert_exchange(data_type::Exchange field)
    {
        switch (field)
        {
            case data_type::Exchange::SHFE: return "SHFE";
            case data_type::Exchange::CFFEX: return "CFFEX";
            case data_type::Exchange::INE: return "INE";
            case data_type::Exchange::DCE: return "DCE";
            case data_type::Exchange::CZCE: return "CZCE";
            case data_type::Exchange::GFEX: return "GFEX";
            default: throw std::runtime_error("unknown exchange");
        }
    }
    constexpr TThostFtdcDirectionType CTPAdapter::convert_direction(data_type::Direction field)
    {
        switch (field)
        {
            case data_type::Direction::LONG: return THOST_FTDC_D_Buy;
            case data_type::Direction::SHORT: return THOST_FTDC_D_Sell;
            default: throw std::runtime_error("unknown direction");
        }
    }
    constexpr TThostFtdcOffsetFlagType CTPAdapter::convert_offset(data_type::Offset field, data_type::Exchange exchange)
    {
        switch (field)
        {
            case data_type::Offset::OPEN: return THOST_FTDC_OF_Open;
            case data_type::Offset::CLOSE: return THOST_FTDC_OF_Close;
            case data_type::Offset::CLOSE_TD: return THOST_FTDC_OF_CloseToday;
            case data_type::Offset::CLOSE_YD: return THOST_FTDC_OF_CloseYesterday;
            default: throw std::runtime_error("unknown offset");
        }
    }
    constexpr data_type::Offset CTPAdapter::convert_offset(TThostFtdcOffsetFlagType field)
    {
        switch (field)
        {
            case THOST_FTDC_OF_Open: return data_type::Offset::OPEN;
            case THOST_FTDC_OF_CloseYesterday: return data_type::Offset::CLOSE_YD;
            case THOST_FTDC_OF_CloseToday: return data_type::Offset::CLOSE_TD;
            case THOST_FTDC_OF_Close: return data_type::Offset::CLOSE;
            default: throw std::runtime_error("unknown ctp offset");
        }
    }
    util::DateTime CTPAdapter::convert_trading_day_to_natural_day(TThostFtdcDateType trading_day, TThostFtdcTimeType update_time, TThostFtdcMillisecType millisec)
    {
        auto datetime = util::DateTime::strptime(
            std::format("{} {}", trading_day, update_time),
            "%Y%m%d %H:%M:%S",
            millisec, 0, 0
        );
        auto hour = datetime.hour();
        auto back_days = 1;
        if (datetime.weekday() == 1)
        {
            // 周五夜盘零点前
            if (hour >= 16 && hour <= 23)
            {
                back_days = 3;
            }
                // 周五夜盘零点后
            else if (hour >= 0 && hour <= 3)
            {
                back_days = 2;
            }
                // 周一白盘
            else back_days = 0;
        }
        else
        {
            // 夜盘零点前
            if (hour >= 16 && hour <= 23)
            {
                back_days = 1;
            }
                // 夜盘零点后
                // 白盘
            else back_days = 0;
        }


        return datetime - util::TimeDelta({.days=back_days});
    }

}