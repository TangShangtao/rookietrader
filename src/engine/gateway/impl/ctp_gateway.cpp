#include "ctp_gateway.h"
#include <cstring>
#include <filesystem>
#include <format>
#include <NanoLogCpp17.h>
#include <limits>
#include "util/str.h"
#include "util/datetime.h"
using namespace NanoLog::LogLevels;
namespace rk::gateway
{
    void CTPTradeHandler::OnFrontConnected() noexcept
    {
        NANO_LOG(NOTICE, "trade front connected");
        if (!_gateway->_td_continue_login) return;
        if (!_gateway->do_auth())
        {
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
        }
    }
    void CTPTradeHandler::OnFrontDisconnected(int nReason) noexcept
    {
        NANO_LOG(NOTICE, "trade front disconnected. reason %i", nReason);
        _gateway->_td_continue_login = false;
        _gateway->_event_loop->push_event(event::EventType::EVENT_TD_DISCONNECTED, std::any());
    }
    void CTPTradeHandler::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(NOTICE, "%s failed. ErrorID: %i", __FUNCTION__, pRspInfo->ErrorID);
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
            return;
        }
        if (!_gateway->do_trader_login())
        {
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
        }
    }
    void CTPTradeHandler::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(ERROR, "%s failed. ErrorID: %i, ErrorMsg: %s", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
            return;
        }
        _gateway->_front_id = pRspUserLogin->FrontID;
        _gateway->_session_id = pRspUserLogin->SessionID;
        if (!_gateway->do_submit_settlement())
        {
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
        }
    }
    void CTPTradeHandler::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        
    }
    void CTPTradeHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(ERROR, "%s failed. ErrorID: %i, ErrorMsg: %s", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
            return;
        }
        {
            auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
            _gateway->_gateway_status = GatewayStatus::TRADE_LOGIN;
        }
        _gateway->_gateway_status_cv.notify_one();
    }
    void CTPTradeHandler::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(WARNING, "order ref %s order insert failed, res %i, msg %s", pInputOrder->OrderRef, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
            _gateway->_event_loop->push_event(
                event::EventType::EVENT_ORDER_ERROR,
                data_type::OrderError{
                    _gateway->_trading_day,
                    static_cast<uint32_t>(std::stoi(pInputOrder->OrderRef)),
                    data_type::ErrorType::ORDER_INSERT_ERROR,
            std::format("ctp error id {}, error msg {}", pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg))
                }
            );
        }
    }
    void CTPTradeHandler::OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(WARNING, "order ref %s cancel failed, res %i", pInputOrderAction->OrderRef, pRspInfo->ErrorID);
            _gateway->_event_loop->push_event(
                event::EventType::EVENT_ORDER_ERROR,
                data_type::OrderError{
                    _gateway->_trading_day,
                    static_cast<uint32_t>(std::stoi(pInputOrderAction->OrderRef)),
                    data_type::ErrorType::ORDER_CANCEL_ERROR,
            std::format("ctp error id {}, error msg {}", pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg))
                }
            );
        }
    }
    void CTPTradeHandler::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(ERROR, "%s failed. ErrorID: %i, ErrorMsg: %s", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
            return;
        }
        if (!pInvestorPosition)
        {
            NANO_LOG(WARNING, "%s data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto symbol = _gateway->_trade_symbol_to_symbol[pInvestorPosition->InstrumentID];
            auto [it, _] = _gateway->_position_data.try_emplace(symbol, std::make_shared<data_type::PositionData>(symbol));
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
                default: NANO_LOG(ERROR, "unknown position direction %c", pInvestorPosition->PosiDirection);
            }
        }
        if (bIsLast)
        {
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::POSITION_DATA_QUERIED;
            }
            _gateway->_gateway_status_cv.notify_one();
        }
    }
    void CTPTradeHandler::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(ERROR, "%s failed. ErrorID: %i, ErrorMsg: %s", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
            return;
        }
        if (!pOrder)
        {
            NANO_LOG(WARNING, "%s data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto order_ref = static_cast<data_type::OrderRef>(std::stoul(pOrder->OrderRef));
            if (order_ref >= _gateway->_order_data.size())
            {
                _gateway->_order_data.resize(order_ref + 1);
            }
            _gateway->_order_data[order_ref] = data_type::OrderData{
                order_ref,
                {
                    _gateway->_trade_symbol_to_symbol[pOrder->InstrumentID],
                    pOrder->LimitPrice,
                    static_cast<uint32_t>(pOrder->VolumeTotal),
                    pOrder->Direction == THOST_FTDC_D_Buy ? data_type::Direction::LONG : data_type::Direction::SHORT,
                    CTPGateway::convert_offset(pOrder->CombOffsetFlag[0]),
                },
                _gateway->_trading_day,
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
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ORDER_DATA_QUERIED;
            }
            _gateway->_gateway_status_cv.notify_one();
        }
    }
    void CTPTradeHandler::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)  noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(ERROR, "%s failed. ErrorID: %i, ErrorMsg: %s", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
            return;
        }
        if (!pTrade)
        {
            NANO_LOG(WARNING, "%s data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto order_ref = static_cast<data_type::OrderRef>(std::stoul(pTrade->OrderRef));
            if (order_ref >= _gateway->_trade_data.size())
            {
                _gateway->_trade_data.resize(order_ref + 1);
            }
            _gateway->_trade_data[order_ref].emplace_back(data_type::TradeData{
                order_ref,
                pTrade->TradeID,
                pTrade->Price,
                static_cast<uint32_t>(pTrade->Volume),
                _gateway->_trading_day,
                util::DateTime::strptime(
                    std::format("{} {}", pTrade->TradeDate, pTrade->TradeTime),
                    "%Y%m%d %H:%M:%S"
                ),
                0
            });
        }
        if (bIsLast)
        {
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::TRADE_DATA_QUERIED;
            }
            _gateway->_gateway_status_cv.notify_one();
        }
    }
    void CTPTradeHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(ERROR, "%s failed. ErrorID: %i, ErrorMsg: %s", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
            return;
        }
        if (!pTradingAccount)
        {
            NANO_LOG(WARNING, "%s data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            _gateway->_account_data.trading_day = _gateway->_trading_day;
            _gateway->_account_data.update_time = util::DateTime::now();
            _gateway->_account_data.balance = pTradingAccount->Balance;
            _gateway->_account_data.pre_balance = pTradingAccount->PreBalance;
            _gateway->_account_data.deposit = pTradingAccount->Deposit;
            _gateway->_account_data.withdraw = pTradingAccount->Withdraw;
            _gateway->_account_data.margin = pTradingAccount->CurrMargin;
            _gateway->_account_data.pre_margin = pTradingAccount->PreMargin;
            _gateway->_account_data.frozen_margin = pTradingAccount->FrozenMargin;
            _gateway->_account_data.commission = pTradingAccount->Commission;
            _gateway->_account_data.available = pTradingAccount->Available;

        }
        if (bIsLast)
        {
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ACCOUNT_DATA_QUERIED;
            }
            _gateway->_gateway_status_cv.notify_one();
        }
    }
    void CTPTradeHandler::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(WARNING, "error_id: %i", pRspInfo->ErrorID);
        }
    }
    void CTPTradeHandler::OnRtnOrder(CThostFtdcOrderField* pOrder) noexcept
    {
        if (!pOrder) return;
        if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled && std::strlen(pOrder->CancelTime) == 0)
        {
            auto error_msg = util::gbk2utf8(pOrder->StatusMsg);
            NANO_LOG(WARNING, "order ref %s order insert rejected, reason %s", pOrder->OrderRef, error_msg.c_str());
            _gateway->_event_loop->push_event(
                event::EventType::EVENT_ORDER_ERROR,
                data_type::OrderError{
                    _gateway->_trading_day,
                    static_cast<uint32_t>(std::stoi(pOrder->OrderRef)),
                    data_type::ErrorType::ORDER_INSERT_ERROR,
                    std::format("ctp order insert rejected, reason: {}", std::move(error_msg))
                }
            );
            return;
        }
        if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
        {
            auto cancel_time = util::DateTime::strptime(
    std::format("{} {}", pOrder->TradingDay, pOrder->CancelTime),
            "%Y%m%d %H:%M:%S",
        0, 0, 0
            );
            _gateway->_event_loop->push_event(
                event::EventType::EVENT_CANCEL_DATA,
                data_type::CancelData{
                    static_cast<data_type::OrderRef>(std::stoul(pOrder->OrderRef)),
                    static_cast<uint32_t>(pOrder->VolumeTotal - pOrder->VolumeTraded),
                    static_cast<uint32_t>(std::stoi(pOrder->TradingDay)),
                    cancel_time,
                }
            );
            return;
        }

    }
    void CTPTradeHandler::OnRtnTrade(CThostFtdcTradeField *pTrade) noexcept
    {
        if (!pTrade) return;

        _gateway->_event_loop->push_event(
            event::EventType::EVENT_TRADE_DATA,
            data_type::TradeData{
                static_cast<data_type::OrderRef>(std::stoul(pTrade->OrderRef)),
                pTrade->TradeID,
                pTrade->Price,
                static_cast<uint32_t>(pTrade->Volume),
                static_cast<uint32_t>(std::stoi(pTrade->TradingDay)),
                util::DateTime::strptime(
                    std::format("{} {}", pTrade->TradeDate, pTrade->TradeTime),
                    "%Y%m%d %H:%M:%S",
                    0, 0, 0
                ),
                0
            }
        );
    }
    void CTPTradeHandler::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(ERROR, "%s failed. ErrorID: %i, ErrorMsg: %s", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
            return;
        }
        if (!pInstrument)
        {
            NANO_LOG(WARNING, "%s data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto product_class = CTPGateway::convert_product_class(pInstrument->ProductClass);
            if (
                std::find(_gateway->_product_class.begin(),_gateway->_product_class.end(),product_class) !=
                _gateway->_product_class.end()
            )
            {
                auto symbol = data_type::Symbol{
                    pInstrument->InstrumentID,
                    pInstrument->InstrumentID,
                    CTPGateway::convert_exchange(pInstrument->ExchangeID),
                    product_class
                };
                _gateway->_trade_symbol_to_symbol.emplace(symbol.trade_symbol, symbol);
                auto symbol_detail = std::make_shared<data_type::SymbolDetail>(
                    symbol,
                    product_class,
                    pInstrument->UnderlyingInstrID,
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
                _gateway->_underlying_to_symbol_details.emplace(pInstrument->UnderlyingInstrID, symbol_detail);
                _gateway->_symbol_details.emplace(symbol, symbol_detail);
            }

        }
        if (bIsLast)
        {
            NANO_LOG(NOTICE, "query symbol finished, start query symbol detail...");
            CThostFtdcQryInstrumentCommissionRateField req{};
            std::strncpy(req.BrokerID, _gateway->_config.broker_id.c_str(), sizeof(req.BrokerID));
            std::strncpy(req.InvestorID, _gateway->_config.user_id.c_str(), sizeof(req.InvestorID));
            auto res = _gateway->_td_api->ReqQryInstrumentCommissionRate(&req, ++_gateway->_req_id);
            if (res != 0)
            {
                NANO_LOG(ERROR, "query symbol detail return %i", res);
                {
                    auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                    _gateway->_gateway_status = GatewayStatus::ERROR;
                }
                _gateway->_gateway_status_cv.notify_one();
            }
        }

    }
    void CTPTradeHandler::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(ERROR, "%s failed. ErrorID: %i, ErrorMsg: %s", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
            return;
        }
        if (!pInstrumentCommissionRate)
        {
            NANO_LOG(WARNING, "data ptr is nullptr");
        }
        else
        {
            // 手续费设置到品种级别
            if (
                _gateway->_underlying_to_symbol_details.find(pInstrumentCommissionRate->InstrumentID) !=
                _gateway->_underlying_to_symbol_details.end()
            )
            {
                auto range = _gateway->_underlying_to_symbol_details.equal_range(pInstrumentCommissionRate->InstrumentID);
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
                auto it = _gateway->_symbol_details.find(_gateway->_trade_symbol_to_symbol[pInstrumentCommissionRate->InstrumentID]);
                it != _gateway->_symbol_details.end()
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
                NANO_LOG(WARNING, "%s %s symbol detail not found in map", pInstrumentCommissionRate->InstrumentID, pInstrumentCommissionRate->ExchangeID);
            }
        }
        if (bIsLast)
        {
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::SYMBOL_DETAIL_QUERIED;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1)); // 防止查询超过柜台流控
            _gateway->_gateway_status_cv.notify_one();
        }
    }
    void CTPMarketHandler::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(ERROR, "%s failed. ErrorID: %i, ErrorMsg: %s", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
        }
    }
    void CTPMarketHandler::OnFrontConnected() noexcept
    {
        NANO_LOG(NOTICE, "market front connected");
        if (!_gateway->_md_continue_login) return;
        if (!_gateway->do_market_login())
        {
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
        }
    }
    void CTPMarketHandler::OnFrontDisconnected(int nReason) noexcept
    {
        NANO_LOG(WARNING, "market front disconnected. reason %i", nReason);
        _gateway->_md_continue_login = false;
        _gateway->_event_loop->push_event(event::EventType::EVENT_MD_DISCONNECTED, std::any());

    }
    void CTPMarketHandler::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(ERROR, "%s failed. ErrorID: %i, ErrorMsg: %s", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
            return;
        }
        {
            auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
            _gateway->_gateway_status = GatewayStatus::MARKET_LOGIN;
        }
        _gateway->_gateway_status_cv.notify_one();
    }
    void CTPMarketHandler::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) noexcept
    {

    }

    void CTPMarketHandler::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) noexcept
    {
        if (pRspInfo && pRspInfo->ErrorID != 0)
        {
            NANO_LOG(ERROR, "%s failed. ErrorID: %i, ErrorMsg: %s", __FUNCTION__, pRspInfo->ErrorID, util::gbk2utf8(pRspInfo->ErrorMsg).c_str());
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::ERROR;
            }
            _gateway->_gateway_status_cv.notify_one();
            return;
        }
        if (bIsLast)
        {
            {
                auto lock = std::unique_lock(_gateway->_gateway_status_mutex);
                _gateway->_gateway_status = GatewayStatus::SUBSCRIBED;
            }
            _gateway->_gateway_status_cv.notify_one();
        }
    }

    void CTPMarketHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData) noexcept
    {
        if (!pDepthMarketData) return;
        auto symbol = _gateway->_trade_symbol_to_symbol[pDepthMarketData->InstrumentID];
        _gateway->_event_loop->push_event(
            event::EventType::EVENT_TICK_DATA,
            data_type::TickData{
                symbol,
                static_cast<uint32_t>(std::stoul(pDepthMarketData->TradingDay)),
                CTPGateway::convert_trading_day_to_natural_day(pDepthMarketData->TradingDay, pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec),
                pDepthMarketData->LastPrice,
                static_cast<uint32_t>(pDepthMarketData->Volume),
                pDepthMarketData->OpenInterest,
                pDepthMarketData->AveragePrice,
                {
                    {pDepthMarketData->BidPrice1, pDepthMarketData->BidVolume1},
                    {pDepthMarketData->BidPrice2, pDepthMarketData->BidVolume2},
                    {pDepthMarketData->BidPrice3, pDepthMarketData->BidVolume3},
                    {pDepthMarketData->BidPrice4, pDepthMarketData->BidVolume4},
                    {pDepthMarketData->BidPrice5, pDepthMarketData->BidVolume5},
                },
                {
                    {pDepthMarketData->AskPrice1, pDepthMarketData->AskVolume1},
                    {pDepthMarketData->AskPrice2, pDepthMarketData->AskVolume2},
                    {pDepthMarketData->AskPrice3, pDepthMarketData->AskVolume3},
                    {pDepthMarketData->AskPrice4, pDepthMarketData->AskVolume4},
                    {pDepthMarketData->AskPrice5, pDepthMarketData->AskVolume5},
                }
            }
        );
    }
    CTPGateway::CTPGateway(std::shared_ptr<event::EventLoop> event_loop, config_type::GatewayConfig config)
        :
    Gateway(std::move(event_loop), std::move(config)),
    _td_handler(this),
    _md_handler(this),
    _order_ref_to_trade_symbol(1000)
    {

    }
    bool CTPGateway::login_trade()
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
        _td_api->RegisterSpi(&_td_handler);
        _td_api->RegisterFront(const_cast<char*>(std::format("{}://{}:{}", _config.sock_type, _config.trade_front_ip, _config.trade_front_port).c_str()));
        _td_api->SubscribePrivateTopic(THOST_TE_RESUME_TYPE::THOST_TERT_QUICK);
        NANO_LOG(NOTICE, "start login trade front(%s:%s)...", _config.trade_front_ip.c_str(), _config.trade_front_port.c_str());
        auto status_lock = std::unique_lock(_gateway_status_mutex);
        _td_continue_login = true;
        _td_api->Init();
        if (
            !_gateway_status_cv.wait_for(
                status_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return (_gateway_status == GatewayStatus::TRADE_LOGIN) |
                           (_gateway_status == GatewayStatus::ERROR) ;
                }
            )
        )
        {
            NANO_LOG(ERROR, "login trade front time out");
            return false;
        }
        if (_gateway_status == GatewayStatus::ERROR)
        {
            NANO_LOG(ERROR, "login trade front error");
            return false;
        }
        NANO_LOG(NOTICE, "login trade front success");
        return true;
    }
    bool CTPGateway::login_market()
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
        NANO_LOG(NOTICE, "start login market front(%s:%s)...", _config.market_front_ip.c_str(), _config.market_front_port.c_str());
        auto status_lock = std::unique_lock(_gateway_status_mutex);
        _md_continue_login = true;
        _md_api->Init();
        if (
            !_gateway_status_cv.wait_for(
                status_lock,
                std::chrono::seconds(30),
                [&]()
                {
                    return (_gateway_status == GatewayStatus::MARKET_LOGIN) |
                           (_gateway_status == GatewayStatus::ERROR) ;
                }
            )
            )
        {
            NANO_LOG(ERROR, "login market front time out");
            return false;
        }
        if (_gateway_status == GatewayStatus::ERROR)
        {
            NANO_LOG(ERROR, "login market front error");
            return false;
        }
        NANO_LOG(NOTICE, "login market front success");
        return true;
    }
    void CTPGateway::logout_trade()
    {
        CThostFtdcUserLogoutField field{};
        std::strncpy(field.BrokerID, _config.broker_id.c_str(), sizeof(field.BrokerID));
        std::strncpy(field.UserID, _config.user_id.c_str(), sizeof(field.UserID));
        _td_api->ReqUserLogout(&field, ++_req_id);
        _td_api = nullptr;
        _gateway_status = GatewayStatus::UNKNOWN;
        NANO_LOG(NOTICE, "logout from trade front");
    }
    void CTPGateway::logout_market()
    {
        _md_api = nullptr;
        _gateway_status = GatewayStatus::UNKNOWN;
        NANO_LOG(NOTICE, "logout from market front");
    }
    bool CTPGateway::subscribe(std::unordered_set<data_type::Symbol> symbols)
    {
        NANO_LOG(NOTICE, "start subscribe symbol(num: %lu) market data...", symbols.size());
        auto status_lock = std::unique_lock(_gateway_status_mutex);
        if (!do_subscribe(symbols))
        {
            NANO_LOG(ERROR, "subscribe error");
            return false;
        }
        if (
            !_gateway_status_cv.wait_for(
                status_lock,
                std::chrono::seconds(10),
                [&]()
                {
                    return (_gateway_status == GatewayStatus::SUBSCRIBED) |
                           (_gateway_status == GatewayStatus::ERROR) ;
                }
            )
            )
        {
            NANO_LOG(ERROR, "subscribe time out");
            return false;
        }
        if (_gateway_status == GatewayStatus::ERROR)
        {
            NANO_LOG(ERROR, "subscribe error");
            return false;
        }
        NANO_LOG(NOTICE, "subscribe success");
        _gateway_status = GatewayStatus::MARKET_LOGIN;
        return true;
    }
    uint32_t CTPGateway::query_trading_day()
    {
        _trading_day = std::stol(_td_api->GetTradingDay());
        return _trading_day;
    }
    std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>>> CTPGateway::query_symbol_details()
    {
        NANO_LOG(NOTICE, "start query symbol...");
        _symbol_details.clear();
        CThostFtdcQryInstrumentField req{};
        auto status_lock = std::unique_lock(_gateway_status_mutex);
        auto res = _td_api->ReqQryInstrument(&req, ++_req_id);
        if (res != 0)
        {
            NANO_LOG(ERROR, "query symbol req return %i", res);
            return std::nullopt;
        }
        if (
            !_gateway_status_cv.wait_for(
                status_lock,
                std::chrono::seconds(60),
                [&]()
                {
                    return (_gateway_status == GatewayStatus::SYMBOL_DETAIL_QUERIED) |
                           (_gateway_status == GatewayStatus::ERROR) ;
                }
            )
        )
        {
            NANO_LOG(ERROR, "query symbol detail time out");
            return std::nullopt;
        }
        if (_gateway_status == GatewayStatus::ERROR)
        {
            NANO_LOG(ERROR, "query symbol detail error");
            return std::nullopt;
        }
        _gateway_status = GatewayStatus::MARKET_LOGIN;
        NANO_LOG(NOTICE, "query symbol detail finished, symbol num: %lu", _symbol_details.size());
        return std::move(_symbol_details);

    }
    std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::PositionData>>> CTPGateway::query_position_data()
    {
        NANO_LOG(NOTICE, "start query position...");
        _position_data.clear();
        CThostFtdcQryInvestorPositionField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID));
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID));
        auto status_lock = std::unique_lock(_gateway_status_mutex);
        auto res = _td_api->ReqQryInvestorPosition(&req, ++_req_id);
        if (res != 0)
        {
            NANO_LOG(ERROR, "query position return %i", res);
            return std::nullopt;
        }
        if (
            !_gateway_status_cv.wait_for(
                status_lock,
                std::chrono::seconds(10),
                [&]()
                {
                    return (_gateway_status == GatewayStatus::POSITION_DATA_QUERIED) |
                           (_gateway_status == GatewayStatus::ERROR) ;
                }
            )
        )
        {
            NANO_LOG(ERROR, "query position data time out");
            return std::nullopt;
        }
        if (_gateway_status == GatewayStatus::ERROR)
        {
            NANO_LOG(ERROR, "query position data error");
            return std::nullopt;
        }
        _gateway_status = GatewayStatus::MARKET_LOGIN;
        NANO_LOG(NOTICE, "query positon finished, symbol num: %lu", _position_data.size());
        return std::move(_position_data);

    }
    std::optional<std::vector<data_type::OrderData>> CTPGateway::query_order_data()
    {
        NANO_LOG(NOTICE, "start query order data...");
        _order_data.clear();
        CThostFtdcQryOrderField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID));
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID));
        auto status_lock = std::unique_lock(_gateway_status_mutex);
        auto res = _td_api->ReqQryOrder(&req, ++_req_id);
        if (res != 0)
        {
            NANO_LOG(ERROR, "query order return %i", res);
            return std::nullopt;
        }
        if (
            !_gateway_status_cv.wait_for(
                status_lock,
                std::chrono::seconds(10),
                [&]()
                {
                    return (_gateway_status == GatewayStatus::ORDER_DATA_QUERIED) |
                           (_gateway_status == GatewayStatus::ERROR) ;
                }
            )
            )
        {
            NANO_LOG(ERROR, "query order data time out");
            return std::nullopt;
        }
        if (_gateway_status == GatewayStatus::ERROR)
        {
            NANO_LOG(ERROR, "query order data error");
            return std::nullopt;
        }
        _gateway_status = GatewayStatus::MARKET_LOGIN;
        NANO_LOG(NOTICE, "query order data finished, order num: %lu", _order_data.size());
        return std::move(_order_data);
    }
    std::optional<std::vector<std::vector<data_type::TradeData>>> CTPGateway::query_trade_data()
    {
        NANO_LOG(NOTICE, "start query trade data...");
        _trade_data.clear();
        CThostFtdcQryTradeField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID));
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID));
        auto status_lock = std::unique_lock(_gateway_status_mutex);
        auto res = _td_api->ReqQryTrade(&req, ++_req_id);
        if (res != 0)
        {
            NANO_LOG(ERROR, "query trade return %i", res);
            return std::nullopt;
        }
        if (
            !_gateway_status_cv.wait_for(
                status_lock,
                std::chrono::seconds(10),
                [&]()
                {
                    return (_gateway_status == GatewayStatus::TRADE_DATA_QUERIED) |
                           (_gateway_status == GatewayStatus::ERROR) ;
                }
            )
            )
        {
            NANO_LOG(ERROR, "query trade data time out");
            return std::nullopt;
        }
        if (_gateway_status == GatewayStatus::ERROR)
        {
            NANO_LOG(ERROR, "query trade data error");
            return std::nullopt;
        }
        _gateway_status = GatewayStatus::MARKET_LOGIN;
        NANO_LOG(NOTICE, "query trade data finished, trade num: %lu", _trade_data.size());
        return std::move(_trade_data);
    }
    std::optional<data_type::AccountData> CTPGateway::query_account_data()
    {
        NANO_LOG(NOTICE, "start query account data...");
        CThostFtdcQryTradingAccountField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID));
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID));
        auto status_lock = std::unique_lock(_gateway_status_mutex);
        auto res = _td_api->ReqQryTradingAccount(&req, ++_req_id);
        if (res != 0)
        {
            NANO_LOG(ERROR, "query account return %i", res);
            return std::nullopt;
        }
        if (
            !_gateway_status_cv.wait_for(
                status_lock,
                std::chrono::seconds(10),
                [&]()
                {
                    return (_gateway_status == GatewayStatus::ACCOUNT_DATA_QUERIED) |
                           (_gateway_status == GatewayStatus::ERROR) ;
                }
            )
            )
        {
            NANO_LOG(ERROR, "query account data time out");
            return std::nullopt;
        }
        if (_gateway_status == GatewayStatus::ERROR)
        {
            NANO_LOG(ERROR, "query account data error");
            return std::nullopt;
        }
        _gateway_status = GatewayStatus::MARKET_LOGIN;
        return std::move(_account_data);
    }
    void CTPGateway::order_insert(data_type::OrderRef order_ref, const data_type::OrderReq& order_req)
    {
        CThostFtdcInputOrderField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID));
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID));
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID));
        std::sprintf(req.OrderRef, "%u", order_ref);
        req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
        req.Direction = convert_direction(order_req.direction);
        req.CombOffsetFlag[0] = convert_offset(order_req.offset, order_req.symbol.exchange);
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
        std::strncpy(req.ExchangeID, convert_exchange(order_req.symbol.exchange).data(), sizeof(req.ExchangeID));
        std::strncpy(req.InstrumentID, order_req.symbol.trade_symbol.c_str(), sizeof(req.InstrumentID));
        req.UserForceClose = 0;
        auto res = _td_api->ReqOrderInsert(&req, ++_req_id);
        if (res != 0)
        {
            NANO_LOG(ERROR, "order insert return %i", res);
            _event_loop->push_event(
                event::EventType::EVENT_ORDER_ERROR,
                data_type::OrderError{
                    _trading_day,
                    order_ref,
                    data_type::ErrorType::ORDER_INSERT_ERROR,
            std::format("api return {}", res)
                }
            );
        }
    }
    void CTPGateway::order_cancel(data_type::OrderRef order_ref)
    {
        CThostFtdcInputOrderActionField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID));
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID));
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID));
        std::strncpy(req.InstrumentID, _order_ref_to_trade_symbol[order_ref].c_str(), sizeof(req.InstrumentID));
        std::sprintf(req.OrderRef, "%u", order_ref);
        req.FrontID = _front_id;
        req.SessionID = _session_id;
        req.ActionFlag = THOST_FTDC_AF_Delete;
        auto res = _td_api->ReqOrderAction(&req, ++_req_id);
        if (res)
        {
            NANO_LOG(ERROR, "order cancel return %i", res);
            _event_loop->push_event(
                event::EventType::EVENT_ORDER_ERROR,
                data_type::OrderError{
                    _trading_day,
                    order_ref,
                    data_type::ErrorType::ORDER_CANCEL_ERROR,
            std::format("api return {}", res)
                }
            );
        }
    }

    bool CTPGateway::do_auth()
    {
        CThostFtdcReqAuthenticateField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID));
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID));
        std::strncpy(req.AuthCode, _config.auth_code.c_str(), sizeof(req.AuthCode));
        std::strncpy(req.AppID, _config.app_id.c_str(), sizeof(req.AppID));
        auto res = _td_api->ReqAuthenticate(&req, ++_req_id);
        if (res != 0)
        {
            NANO_LOG(ERROR, "req auth return %i", res);
            return false;
        }
        return true ;
    }
    bool CTPGateway::do_trader_login()
    {
        CThostFtdcReqUserLoginField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID));
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID));
        std::strncpy(req.Password, _config.password.c_str(), sizeof(req.Password));
        auto res = _td_api->ReqUserLogin(&req, ++_req_id);
        if (res != 0)
        {
            NANO_LOG(ERROR, "login trade front return %i", res);
            return false;
        }
        return true;
    }
    bool CTPGateway::do_submit_settlement()
    {
        CThostFtdcSettlementInfoConfirmField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID));
        std::strncpy(req.InvestorID, _config.user_id.c_str(), sizeof(req.InvestorID));

        auto res = _td_api->ReqSettlementInfoConfirm(&req, ++_req_id);
        if (res != 0)
        {
            NANO_LOG(ERROR, "req settlement confirm return %i", res);
            return false;
        }
        return true;
    }
    bool CTPGateway::do_market_login()
    {
        CThostFtdcReqUserLoginField req{};
        std::strncpy(req.BrokerID, _config.broker_id.c_str(), sizeof(req.BrokerID));
        std::strncpy(req.UserID, _config.user_id.c_str(), sizeof(req.UserID));
        std::strncpy(req.Password, _config.password.c_str(), sizeof(req.Password));
        auto res = _md_api->ReqUserLogin(&req, ++_req_id);
        if (res != 0)
        {
            NANO_LOG(ERROR, "login market front return %i", res);
            return false;
        }
        return true;
    }
    bool CTPGateway::do_subscribe(const std::unordered_set<data_type::Symbol>& symbols)
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
            NANO_LOG(ERROR, "subscribe return %i", res);
            return false;
        }
        return true;
    }
    data_type::ProductClass CTPGateway::convert_product_class(TThostFtdcProductClassType counter_field)
    {
        switch (counter_field)
        {
            case THOST_FTDC_PC_Futures: return data_type::ProductClass::FUTURE;
            default: return data_type::ProductClass::UNKNOWN;
        }
    }
    data_type::Exchange CTPGateway::convert_exchange(TThostFtdcExchangeIDType counter_field)
    {
        if (std::strcmp(counter_field, "SHFE") == 0) return data_type::Exchange::SHFE;
        if (std::strcmp(counter_field, "CFFEX") == 0) return data_type::Exchange::CFFEX;
        if (std::strcmp(counter_field, "INE") == 0) return data_type::Exchange::INE;
        if (std::strcmp(counter_field, "DCE") == 0) return data_type::Exchange::DCE;
        if (std::strcmp(counter_field, "CZCE") == 0) return data_type::Exchange::CZCE;
        if (std::strcmp(counter_field, "GFEX") == 0) return data_type::Exchange::GFEX;
        return data_type::Exchange::UNKNOWN;
    }
    constexpr std::string_view CTPGateway::convert_exchange(data_type::Exchange field)
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
    constexpr TThostFtdcDirectionType CTPGateway::convert_direction(data_type::Direction field)
    {
        switch (field)
        {
            case data_type::Direction::LONG: return THOST_FTDC_D_Buy;
            case data_type::Direction::SHORT: return THOST_FTDC_D_Sell;
            default: throw std::runtime_error("unknown direction");
        }
    }
    constexpr TThostFtdcOffsetFlagType CTPGateway::convert_offset(data_type::Offset field, data_type::Exchange exchange)
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
    constexpr data_type::Offset CTPGateway::convert_offset(TThostFtdcOffsetFlagType field)
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
    util::DateTime CTPGateway::convert_trading_day_to_natural_day(TThostFtdcDateType trading_day, TThostFtdcTimeType update_time, TThostFtdcMillisecType millisec)
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

};