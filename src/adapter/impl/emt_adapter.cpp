#include "emt_adapter.h"
#include "util/logger.h"
#include "util/str.h"
#include <magic_enum/magic_enum.hpp>
namespace rk::adapter
{
    EMTTDAdapter::EMTTDAdapter(TDAdapter::PushDataCallbacks push_data_callbacks, config_type::TDAdapterConfig config)
    :
        TDAdapter(std::move(push_data_callbacks), std::move(config)), _order_ref_to_emt_order_id(1000)
    {

    }
    bool EMTTDAdapter::login()
    {
        const auto trade_flow_path = std::format("emt_trade_flow/{}/", _config.user_id);
        if (!std::filesystem::exists(trade_flow_path))
        {
            std::filesystem::create_directories(trade_flow_path);
        }
        _td_api = std::unique_ptr<EMT::API::TraderApi, EMTTDDeleter>(
            EMT::API::TraderApi::CreateTraderApi(
                1,
                trade_flow_path.c_str(),
                EMT_LOG_LEVEL::EMT_LOG_LEVEL_INFO
            ),
            EMTTDDeleter()
        );
        _td_api->RegisterSpi(this);
        _td_api->SubscribePublicTopic(EMT_TE_RESUME_TYPE::EMT_TERT_QUICK);
        RK_LOG_INFO("start login trade front...");
        _session_id = _td_api->Login(
            _config.trade_front_ip.c_str(),
            std::stoi(_config.trade_front_port),
            _config.user_id.c_str(),
            _config.password.c_str(),
            _config.sock_type == "tcp" ? EMT_PROTOCOL_TYPE::EMT_PROTOCOL_TCP : EMT_PROTOCOL_TYPE::EMT_PROTOCOL_UDP
        );
        if (_session_id == 0)
        {
            auto error = _td_api->GetApiLastError();
            RK_LOG_ERROR("login trade front error, error_id: {}, error_msg: {}", error->error_id, error->error_msg);
            return false;
        }
        return true;
    }
    void EMTTDAdapter::logout()
    {
        _td_api->Logout(_session_id);
        _td_api = nullptr;
    }
    uint32_t EMTTDAdapter::query_trading_day()
    {
        _trading_day = std::stoul(_td_api->GetTradingDay());
        return _trading_day;
    }
    std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::PositionData>>> EMTTDAdapter::query_position_data()
    {
        _position_data.clear();
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        auto field = EMTQueryByPageReq{200, 0};
        auto res = _td_api->QueryPositionByPage(&field, _session_id, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("query position return {}", res);
            return std::nullopt;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(10),
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
    std::optional<std::vector<data_type::OrderData>> EMTTDAdapter::query_order_data()
    {
        _order_data.clear();
        auto req = EMTQueryByPageReq{200, 0};
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        auto res = _td_api->QueryOrdersByPage(&req, _session_id, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("query order return {}", res);
            return std::nullopt;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(10),
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
        RK_LOG_INFO("query order data finished, order num: {}", _order_data.size());
        return std::move(_order_data);
    }
    std::optional<std::vector<std::vector<data_type::TradeData>>> EMTTDAdapter::query_trade_data()
    {
        RK_LOG_INFO("start query trade data...");
        _trade_data.clear();
        auto req = EMTQueryByPageReq{200, 0};
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        auto res = _td_api->QueryTradesByPage(&req, _session_id, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("query trade return {}", res);
            return std::nullopt;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(10),
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
        RK_LOG_INFO("query trade data finished, trade num: {}", _trade_data.size());
        return std::move(_trade_data);
    }
    std::optional<data_type::AccountData> EMTTDAdapter::query_account_data()
    {
        RK_LOG_INFO("start query account data...");
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        auto res = _td_api->QueryAsset(_session_id, ++_req_id);
        if (res != 0)
        {
            RK_LOG_ERROR("query account return {}", res);
            return std::nullopt;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(10),
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
        RK_LOG_INFO("query account data finished, balance: {}", _account_data.balance);
        return _account_data;
    }
    void EMTTDAdapter::order_insert(data_type::OrderRef order_ref, const data_type::OrderReq& order_req)
    {
        EMTOrderInsertInfo req{};
        req.order_client_id = order_ref;
        std::strncpy(req.ticker, order_req.symbol.trade_symbol.c_str(), sizeof(req.ticker));
        req.market = EMTGateway::convert_exchange(order_req.symbol.exchange);
        req.price = order_req.limit_price;
        req.quantity = order_req.volume;
        req.price_type = EMT_PRICE_TYPE::EMT_PRICE_LIMIT;
        req.side = EMTGateway::convert_direction(order_req.direction);
        req.position_effect = EMTGateway::convert_offset(order_req.offset);
        req.business_type = EMTGateway::convert_business_type(order_req.direction);
        auto emt_order_ref = _td_api->InsertOrder(&req, _session_id);
        if (emt_order_ref == 0)
        {
            auto error = _td_api->GetApiLastError();
            RK_LOG_ERROR("order insert error, error_id: {}, error_msg: {}", error->error_id, error->error_msg);
            _push_data_callbacks.push_order_error({
                _trading_day,
                order_ref,
                data_type::ErrorType::ORDER_INSERT_ERROR,
                std::format("order insert error, error_id: {}, error_msg: {}", error->error_id, error->error_msg)
            });
        }
        _order_ref_to_emt_order_id[order_ref] = emt_order_ref;

    }
    void EMTTDAdapter::order_cancel(data_type::OrderRef order_ref)
    {
        auto order_emt_id = _order_ref_to_emt_order_id[order_ref];
        auto res = _td_api->CancelOrder(order_emt_id, _session_id);
        if (res == 0)
        {
            auto error = _td_api->GetApiLastError();
            RK_LOG_ERROR("order cancel error, error_id: {}, error_msg: {}", error->error_id, error->error_msg);
            _push_data_callbacks.push_order_error({
                _trading_day,
                order_ref,
                data_type::ErrorType::ORDER_INSERT_ERROR,
                std::format("order insert error, error_id: {}, error_msg: {}", error->error_id, error->error_msg)
            });
            return;
        }
        _emt_order_id_to_order_ref[order_emt_id] = order_ref;
    }
    void EMTTDAdapter::notify_rpc_result(RPCResult result)
    {
        {
            auto lock = std::unique_lock(_rpc_mutex);
            _rpc_result = result;
        }
        _rpc_condition_variable.notify_one();
    }
    void EMTTDAdapter::OnConnected()
    {
        RK_LOG_INFO("connected to trade front");
    }
    void EMTTDAdapter::OnDisconnected(int reason)
    {
        RK_LOG_WARN("disconnected to trade front, reason: {}", reason);
    }
    void EMTTDAdapter::OnOrderEvent(EMTOrderInfo *order_info, EMTRI *error_info, uint64_t session_id)
    {
        if (!order_info) return;
        if (error_info && error_info->error_id != 0)
        {
            RK_LOG_WARN("order ref %ul order insert rejected, reason {}", order_info->order_client_id, error_info->error_msg);
            _push_data_callbacks.push_order_error({
                _trading_day,
                order_info->order_client_id,
                data_type::ErrorType::ORDER_INSERT_ERROR,
                std::format("emt order insert rejected, reason: {}", error_info->error_msg)
            });
            return;
        }
        if (
            order_info->order_status == EMT_ORDER_STATUS_TYPE::EMT_ORDER_STATUS_PARTTRADEDNOTQUEUEING ||
            order_info->order_status == EMT_ORDER_STATUS_TYPE::EMT_ORDER_STATUS_CANCELED
            )
        {
            auto cancel_time = EMTGateway::convert_datetime(order_info->cancel_time);
            _push_data_callbacks.push_cancel({
                order_info->order_client_id,
                static_cast<uint32_t>(order_info->qty_left),
                _trading_day,
                cancel_time
            });
            return;
        }

    }
    void EMTTDAdapter::OnTradeEvent(EMTTradeReport *trade_info, uint64_t session_id)
    {
        if (!trade_info) return;
        auto trade_time = EMTGateway::convert_datetime(trade_info->trade_time);
        _push_data_callbacks.push_trade({
            trade_info->order_client_id,
            trade_info->exec_id,
            trade_info->price,
            static_cast<uint32_t>(trade_info->quantity),
            _trading_day,
            trade_time,
            0
        });
    }
    void EMTTDAdapter::OnCancelOrderError(EMTOrderCancelInfo *cancel_info, EMTRI *error_info, uint64_t session_id)
    {
        if (error_info && error_info->error_id != 0)
        {
            if (_emt_order_id_to_order_ref.find(cancel_info->order_emt_id) == _emt_order_id_to_order_ref.end()) return;
            auto order_ref = _emt_order_id_to_order_ref[cancel_info->order_emt_id];
            RK_LOG_WARN("order ref {} cancel failed, error_id: {}, error_msg: {}",order_ref,error_info->error_id, error_info->error_msg);
            _push_data_callbacks.push_order_error({
                _trading_day,
                order_ref,
                data_type::ErrorType::ORDER_CANCEL_ERROR,
                std::format("emt error id: {}, error msg: {}", error_info->error_id, error_info->error_msg)
            });
        }
    }
    void EMTTDAdapter::OnQueryPositionByPage(EMTQueryStkPositionRsp *trade_info, int64_t req_count, int64_t trade_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id)
    {
        if (!trade_info)
        {
            RK_LOG_WARN("{} data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto exchange = EMTGateway::convert_exchange(trade_info->market);
            if (exchange == data_type::Exchange::UNKNOWN) return;
            auto symbol = data_type::Symbol{EMTGateway::convert_trade_symbol_to_symbol(exchange, trade_info->ticker)};
            if (_position_data.find(symbol) == _position_data.end())
            {
                _position_data.emplace(
                    symbol,
                    std::make_shared<data_type::PositionData>(symbol)
                );
            }
            auto it = _position_data.find(symbol);
            it->second->long_position.position += trade_info->total_qty;
        }
        if (is_last)
        {
            if (trade_sequence == req_count)
            {
                auto field = EMTQueryByPageReq{200, query_reference};
                auto res = _td_api->QueryPositionByPage(&field, _session_id, ++_req_id);
                if (res != 0)
                {
                    auto error =  _td_api->GetApiLastError();
                    RK_LOG_ERROR("query position failed! error_id: {}, error_msg: {}", error->error_id, error->error_msg);
                    notify_rpc_result(RPCResult::FAILED);
                }
            }
            else
            {
                notify_rpc_result(RPCResult::SUCCESS);
            }
        }
    }
    void EMTTDAdapter::OnQueryOrderByPage(EMTQueryOrderRsp *order_info, int64_t req_count, int64_t order_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id)
    {
        if (!order_info)
        {
            RK_LOG_WARN("{} data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto order_ref = order_info->order_client_id;
            auto exchange = EMTGateway::convert_exchange(order_info->market);
            if (exchange == data_type::Exchange::UNKNOWN) return;
            if (order_ref >= _order_data.size())
            {
                _order_data.resize(order_ref + 1);
            }
            auto symbol = data_type::Symbol{EMTGateway::convert_trade_symbol_to_symbol(exchange, order_info->ticker)};
            _order_data[order_ref] = data_type::OrderData{
                order_ref,
                {
                    symbol,
                    order_info->price,
                    static_cast<uint32_t>(order_info->quantity),
                    EMTGateway::convert_direction(order_info->side),
                    data_type::Offset::UNKNOWN,
                },
                _trading_day,
                EMTGateway::convert_datetime(order_info->insert_time),
                static_cast<uint32_t>(order_info->qty_traded),
                order_info->cancel_time == 0 ? static_cast<uint32_t>(order_info->qty_left) : 0,
                order_info->cancel_time != 0 ? static_cast<uint32_t>(order_info->qty_left) : 0
            };
        }
        if (is_last)
        {
            if (order_sequence == req_count)
            {
                auto field = EMTQueryByPageReq{200, query_reference};
                auto res = _td_api->QueryOrdersByPage(&field, _session_id, ++_req_id);
                if (res != 0)
                {
                    auto error =  _td_api->GetApiLastError();
                    RK_LOG_ERROR("query order failed! error_id: {}, error_msg: {}", error->error_id, error->error_msg);
                    notify_rpc_result(RPCResult::FAILED);
                }
            }
            else
            {
                notify_rpc_result(RPCResult::SUCCESS);
            }
        }
    }
    void EMTTDAdapter::OnQueryTradeByPage(EMTQueryTradeRsp *trade_info, int64_t req_count, int64_t trade_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id)
    {
        if (!trade_info)
        {
            RK_LOG_WARN("{} data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto order_ref = trade_info->order_client_id;
            if (order_ref >= _trade_data.size())
            {
                _trade_data.resize(order_ref + 1);
            }
            _trade_data[order_ref].emplace_back(data_type::TradeData{
                order_ref,
                trade_info->exec_id,
                trade_info->price,
                static_cast<uint32_t>(trade_info->quantity),
                _trading_day,
                EMTGateway::convert_datetime(trade_info->trade_time),
                0
            });
        }
        if (is_last)
        {
            if (trade_sequence == req_count)
            {
                auto field = EMTQueryByPageReq{200, query_reference};
                auto res = _td_api->QueryTradesByPage(&field, _session_id, ++_req_id);
                if (res != 0)
                {
                    auto error =  _td_api->GetApiLastError();
                    RK_LOG_ERROR("query trade failed! error_id: {}, error_msg: {}", error->error_id, error->error_msg);
                    notify_rpc_result(RPCResult::FAILED);
                }
            }
            else
            {
                notify_rpc_result(RPCResult::SUCCESS);
            }
        }
    }
    void EMTTDAdapter::OnQueryAsset(EMTQueryAssetRsp *asset, EMTRI *error_info, int request_id, bool is_last, uint64_t session_id)
    {
        if (error_info && error_info->error_id != 0)
        {
            RK_LOG_ERROR("{} failed. ErrorID: {}", __FUNCTION__, error_info->error_id);
            notify_rpc_result(RPCResult::FAILED);
            return;
        }
        if (!asset)
        {
            RK_LOG_WARN("{} data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            _account_data.trading_day = _trading_day;
            _account_data.update_time = util::DateTime::now();
            _account_data.balance = asset->total_asset;
            _account_data.market_value = asset->security_asset;
            _account_data.available = asset->buying_power;
        }
        if (is_last)
        {
            notify_rpc_result(RPCResult::SUCCESS);
        }
    }
    EMTMDAdapter::EMTMDAdapter(MDAdapter::PushDataCallbacks push_data_callbacks, config_type::MDAdapterConfig config)
    : MDAdapter(std::move(push_data_callbacks), std::move(config))
    {
        
    }
    bool EMTMDAdapter::login()
    {
        const auto market_flow_path = std::format("emt_market_flow/{}/", _config.user_id);
        if (!std::filesystem::exists(market_flow_path))
        {
            std::filesystem::create_directories(market_flow_path);
        }
        _md_api = std::unique_ptr<EMQ::API::QuoteApi, EMTMDDeleter>(
            EMQ::API::QuoteApi::CreateQuoteApi(
                market_flow_path.c_str(),
                EMQ_LOG_LEVEL::EMQ_LOG_LEVEL_INFO,
                EMQ_LOG_LEVEL::EMQ_LOG_LEVEL_INFO
            ),
            EMTMDDeleter()
        );
        _md_api->RegisterSpi(this);
        auto res = _md_api->Login(
            _config.market_front_ip.c_str(),
            std::stoul(_config.market_front_port),
            _config.user_id.c_str(),
            _config.password.c_str()
        );
        if (res != 0)
        {
            _rpc_result = RPCResult::FAILED;
            RK_LOG_ERROR("login market front error, res: {}", res);
            return false;
        }
        return true;
    }
    void EMTMDAdapter::logout()
    {
        _md_api->Logout();
        _md_api = nullptr;
    }
    bool EMTMDAdapter::subscribe(std::unordered_set<data_type::Symbol> symbols)
    {
        RK_LOG_INFO("start subscribe symbol(num: {}) market data...", symbols.size());
        if (symbols.empty()) return true;
        for (const auto& symbol : symbols)
        {
            _subscribed_symbols.emplace(symbol);
        }
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        if (!do_subscribe(symbols))
        {
            RK_LOG_ERROR("subscribe error");
            return false;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(10),
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
            RK_LOG_ERROR("subscribe data error");
            return false;
        }
        _rpc_result = RPCResult::UNKNOWN;
        RK_LOG_INFO("subscribe succeed!");
        return true;
    }
    bool EMTMDAdapter::unsubscribe(std::unordered_set<data_type::Symbol> symbols)
    {
        RK_LOG_INFO("start unsubscribe symbol(num: {}) market data...", symbols.size());
        if (symbols.empty()) return true;
        for (const auto& symbol : symbols)
        {
            _subscribed_symbols.erase(symbol);
        }
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        if (!do_unsubscribe(symbols))
        {
            RK_LOG_ERROR("unsubscribe error");
            return false;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(10),
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
            RK_LOG_ERROR("unsubscribe data error");
            return false;
        }
        _rpc_result = RPCResult::UNKNOWN;
        RK_LOG_INFO("unsubscribe succeed!");
        return true;
    }
    std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>>> EMTMDAdapter::query_symbol_detail()
    {
        _symbol_detail_query_count = 0;
        RK_LOG_INFO("start query symbol detail...");
        _symbol_detail.clear();
        auto rpc_lock = std::unique_lock(_rpc_mutex);
        auto res = _md_api->QueryAllTickers(EMQ_EXCHANGE_TYPE::EMQ_EXCHANGE_SH);
        if (res != 0)
        {
            RK_LOG_ERROR("query symbol req return {}", res);
            return std::nullopt;
        }
        if (
            !_rpc_condition_variable.wait_for(
                rpc_lock,
                std::chrono::seconds(10),
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
            RK_LOG_ERROR("query account data error");
            return std::nullopt;
        }
        _rpc_result = RPCResult::UNKNOWN;
        return _symbol_detail;
    }

    void EMTMDAdapter::notify_rpc_result(RPCResult result)
    {
        {
            auto lock = std::unique_lock(_rpc_mutex);
            _rpc_result = result;
        }
        _rpc_condition_variable.notify_one();
    }
    bool EMTMDAdapter::do_subscribe(const std::unordered_set<data_type::Symbol>& symbols)
    {
        int sse_num = 0;
        int szse_num = 0;
        for (const auto& it : symbols)
        {
            if (it.exchange == data_type::Exchange::SSE) ++sse_num;
            else if (it.exchange == data_type::Exchange::SZSE) ++szse_num;
        }
        int sse_num_copy = sse_num;
        int szse_num_copy = szse_num;
        char** sse_symbol_list = static_cast<char**>(std::malloc(sse_num * sizeof(char*)));
        char** szse_symbol_list = static_cast<char**>(std::malloc(szse_num * sizeof(char*)));
        for (const auto& it : symbols)
        {
            if (it.exchange == data_type::Exchange::SSE)
                sse_symbol_list[--sse_num] = const_cast<char*>(it.trade_symbol.c_str());
            else if (it.exchange == data_type::Exchange::SZSE)
                szse_symbol_list[--szse_num] = const_cast<char*>(it.trade_symbol.c_str());

        }
        if (sse_num_copy > 0)
        {
            auto res = _md_api->SubscribeMarketData(sse_symbol_list, sse_num_copy, EMQ_EXCHANGE_TYPE::EMQ_EXCHANGE_SH);
            if (res != 0)
            {
                RK_LOG_ERROR("subscribe sse return {}", res);
                return false;
            }
        }
        std::free(sse_symbol_list);
        if (szse_num_copy > 0)
        {
            auto res = _md_api->SubscribeMarketData(szse_symbol_list, szse_num_copy, EMQ_EXCHANGE_TYPE::EMQ_EXCHANGE_SZ);
            if (res != 0)
            {
                RK_LOG_ERROR("subscribe szse return {}", res);
                return false;
            }
        }
        std::free(szse_symbol_list);
        return true;
    }
    bool EMTMDAdapter::do_unsubscribe(const std::unordered_set<data_type::Symbol>& symbols)
    {
        int sse_num = 0;
        int szse_num = 0;
        for (const auto& it : symbols)
        {
            if (it.exchange == data_type::Exchange::SSE) ++sse_num;
            else if (it.exchange == data_type::Exchange::SZSE) ++szse_num;
        }
        int sse_num_copy = sse_num;
        int szse_num_copy = szse_num;
        char** sse_symbol_list = static_cast<char**>(std::malloc(sse_num * sizeof(char*)));
        char** szse_symbol_list = static_cast<char**>(std::malloc(szse_num * sizeof(char*)));
        for (const auto& it : symbols)
        {
            if (it.exchange == data_type::Exchange::SSE)
                sse_symbol_list[--sse_num] = const_cast<char*>(it.trade_symbol.c_str());
            else if (it.exchange == data_type::Exchange::SZSE)
                szse_symbol_list[--szse_num] = const_cast<char*>(it.trade_symbol.c_str());

        }
        auto res = _md_api->UnSubscribeMarketData(sse_symbol_list, sse_num_copy, EMQ_EXCHANGE_TYPE::EMQ_EXCHANGE_SH);
        std::free(sse_symbol_list);
        if (res != 0)
        {
            RK_LOG_ERROR("unsubscribe sse return {}", res);
            return false;
        }
        res = _md_api->UnSubscribeMarketData(szse_symbol_list, szse_num_copy, EMQ_EXCHANGE_TYPE::EMQ_EXCHANGE_SZ);
        std::free(szse_symbol_list);
        if (res != 0)
        {
            RK_LOG_ERROR("unsubscribe szse return {}", res);
            return false;
        }
        return true;
    }
    void EMTMDAdapter::OnQueryAllTickers(EMTQuoteStaticInfo* qsi, EMTRspInfoStruct* error_info, bool is_last)
    {
        if (error_info && error_info->error_id != 0)
        {
            RK_LOG_ERROR("{} failed. error_id: {}, error_msg: {}", __FUNCTION__, error_info->error_id, error_info->error_msg);
            
            return;
        }
        if (!qsi)
        {
            RK_LOG_WARN("{} data ptr is nullptr", __FUNCTION__);
        }
        else
        {
            auto product_class = EMTGateway::convert_product_class(qsi->ticker_type);
            auto exchange = EMTGateway::convert_exchange(qsi->exchange_id);
            if (
                std::find(
                    _config.product_class.begin(),
                    _config.product_class.end(),
                    magic_enum::enum_name(product_class)
                ) != _config.product_class.end() &&
                exchange != data_type::Exchange::UNKNOWN
            )
            {
                auto symbol = data_type::Symbol{
                    EMTGateway::convert_trade_symbol_to_symbol(exchange, qsi->ticker),
                    qsi->ticker,
                    exchange,
                    product_class
                };
                auto symbol_detail = std::make_shared<data_type::SymbolDetail>(
                    symbol,
                    product_class,
                    "",
                    util::gbk2utf8(qsi->ticker_name),
                    qsi->price_tick,
                    1,
                    static_cast<uint32_t>(qsi->buy_qty_unit),
                    static_cast<uint32_t>(qsi->sell_qty_unit),
                    10000,
                    10000,
                    qsi->upper_limit_price,
                    qsi->lower_limit_price,
                    0,
                    0,
                    1,
                    1,
                    0.,0.,
                    0.,0.,
                    0.,0.
                );
                _symbol_detail.emplace(symbol, std::move(symbol_detail));
            }
        }

        if (is_last)
        {
            ++_symbol_detail_query_count;
            if (_symbol_detail_query_count == 2)
            {
                notify_rpc_result(RPCResult::SUCCESS);
            }
            else
            {
                auto res = _md_api->QueryAllTickers(EMQ_EXCHANGE_TYPE::EMQ_EXCHANGE_SZ);
                if (res != 0)
                {
                    notify_rpc_result(RPCResult::FAILED);
                }
            }
        }
    }
    void EMTMDAdapter::OnSubMarketData(EMTSpecificTickerStruct* ticker, EMTRspInfoStruct* error_info, bool is_last)
    {
        if (error_info && error_info->error_id != 0)
        {
            RK_LOG_ERROR("{} failed. error_id: {}, error_msg: {}", __FUNCTION__, error_info->error_id, error_info->error_msg);
            notify_rpc_result(RPCResult::FAILED);
            return;
        }
        if (is_last)
        {
            notify_rpc_result(RPCResult::SUCCESS);
        }
    }
    void EMTMDAdapter::OnUnSubMarketData(EMTSpecificTickerStruct* ticker, EMTRspInfoStruct* error_info, bool is_last)
    {
        if (error_info && error_info->error_id != 0)
        {
            RK_LOG_ERROR("{} failed. error_id: {}, error_msg: {}", __FUNCTION__, error_info->error_id, error_info->error_msg);
            notify_rpc_result(RPCResult::FAILED);
            return;
        }
        if (is_last)
        {
            notify_rpc_result(RPCResult::SUCCESS);
        }
    }
    void EMTMDAdapter::OnDepthMarketData(EMTMarketDataStruct* market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count)
    {
        if (!market_data) return;
        auto exchange = EMTGateway::convert_exchange(market_data->exchange_id);
        if (exchange == data_type::Exchange::UNKNOWN) return;
        auto symbol_name = EMTGateway::convert_trade_symbol_to_symbol(exchange, market_data->ticker);
        if (!_symbol_detail.contains({symbol_name})) return;
        auto symbol = _symbol_detail.find({symbol_name})->first;
        if (_subscribed_symbols.find(symbol) == _subscribed_symbols.end()) return;
        _push_data_callbacks.push_tick({
            symbol,
            static_cast<uint32_t>(market_data->data_time / 1000000000),
            EMTGateway::convert_datetime(market_data->data_time),
            market_data->last_price,
            market_data->open_price,
            market_data->high_price,
            market_data->low_price,
            market_data->upper_limit_price,
            market_data->lower_limit_price,
            static_cast<int64_t>(market_data->qty),
            0,
            market_data->avg_price,
            {
                market_data->bid[0],
                market_data->bid[1],
                market_data->bid[2],
                market_data->bid[3],
                market_data->bid[4],
                market_data->bid[5],
                market_data->bid[6],
                market_data->bid[7],
                market_data->bid[8],
                market_data->bid[9]
            },
            {
                market_data->ask[0],
                market_data->ask[1],
                market_data->ask[2],
                market_data->ask[3],
                market_data->ask[4],
                market_data->ask[5],
                market_data->ask[6],
                market_data->ask[7],
                market_data->ask[8],
                market_data->ask[9]
            },
            {
                market_data->bid_qty[0],
                market_data->bid_qty[1],
                market_data->bid_qty[2],
                market_data->bid_qty[3],
                market_data->bid_qty[4],
                market_data->bid_qty[5],
                market_data->bid_qty[6],
                market_data->bid_qty[7],
                market_data->bid_qty[8],
                market_data->bid_qty[9]
            },
            {
                market_data->ask_qty[0],
                market_data->ask_qty[1],
                market_data->ask_qty[2],
                market_data->ask_qty[3],
                market_data->ask_qty[4],
                market_data->ask_qty[5],
                market_data->ask_qty[6],
                market_data->ask_qty[7],
                market_data->ask_qty[8],
                market_data->ask_qty[9]
            },
            symbol.product_class == data_type::ProductClass::ETF ? market_data->fund.iopv : 0
        });
    }


    data_type::ProductClass EMTGateway::convert_product_class(EMQ_TICKER_TYPE counter_field)
    {
        switch (counter_field)
        {
            case EMQ_TICKER_TYPE_STOCK:
            case EMQ_TICKER_TYPE_TECH_STOCK:
                return data_type::ProductClass::STOCK;
            case EMQ_TICKER_TYPE_FUND:
                return data_type::ProductClass::ETF;
            default:return data_type::ProductClass::UNKNOWN;
        }
    }
    data_type::Exchange EMTGateway::convert_exchange(EMQ_EXCHANGE_TYPE counter_field)
    {
        switch (counter_field)
        {
            case EMQ_EXCHANGE_SH: return data_type::Exchange::SSE;
            case EMQ_EXCHANGE_SZ: return data_type::Exchange::SZSE;
            case EMQ_EXCHANGE_BJGZ: return data_type::Exchange::BSE;
            default:return data_type::Exchange::UNKNOWN;
        }
    }
    data_type::Exchange EMTGateway::convert_exchange(EMT_MARKET_TYPE counter_field)
    {
        switch (counter_field)
        {
            case EMT_MKT_SH_A: return data_type::Exchange::SSE;
            case EMT_MKT_SZ_A: return data_type::Exchange::SZSE;
            case EMT_MKT_BJ_A: return data_type::Exchange::BSE;
            default:return data_type::Exchange::UNKNOWN;
        }
    }
    EMT_MARKET_TYPE EMTGateway::convert_exchange(data_type::Exchange field)
    {
        switch (field)
        {
            case data_type::Exchange::SSE: return EMT_MARKET_TYPE::EMT_MKT_SH_A;
            case data_type::Exchange::SZSE: return EMT_MARKET_TYPE::EMT_MKT_SZ_A;
            default:return EMT_MARKET_TYPE::EMT_MKT_UNKNOWN;
        }
    }
    EMT_SIDE_TYPE EMTGateway::convert_direction(data_type::Direction field)
    {
        switch (field)
        {
            case data_type::Direction::LONG: return EMT_SIDE_BUY;
            case data_type::Direction::SHORT: return EMT_SIDE_SELL;
            default: return EMT_SIDE_UNKNOWN;
        }
    }
    data_type::Direction EMTGateway::convert_direction(EMT_SIDE_TYPE field)
    {
        switch (field)
        {
            case EMT_SIDE_BUY: return data_type::Direction::LONG;
            case EMT_SIDE_SELL: return data_type::Direction::SHORT;
            default: return data_type::Direction::UNKNOWN;
        }
    }
    EMT_POSITION_EFFECT_TYPE EMTGateway::convert_offset(data_type::Offset field)
    {
        switch (field)
        {
            case data_type::Offset::OPEN: return EMT_POSITION_EFFECT_OPEN;
            case data_type::Offset::CLOSE: return EMT_POSITION_EFFECT_CLOSE;
            default: return EMT_POSITION_EFFECT_UNKNOWN;
        }
    }
    data_type::Offset EMTGateway::convert_offset(EMT_POSITION_EFFECT_TYPE field)
    {
        switch (field)
        {
            case EMT_POSITION_EFFECT_OPEN: return data_type::Offset::OPEN;
            case EMT_POSITION_EFFECT_CLOSEYESTERDAY: return data_type::Offset::CLOSE;
            case EMT_POSITION_EFFECT_CLOSETODAY: return data_type::Offset::CLOSE;
            case EMT_POSITION_EFFECT_CLOSE: return data_type::Offset::CLOSE;
            default: return data_type::Offset::UNKNOWN;
        }
    }
    EMT_BUSINESS_TYPE_EXT EMTGateway::convert_business_type(data_type::Direction field)
    {
        switch (field)
        {
            case data_type::Direction::LONG:
            case data_type::Direction::SHORT: return EMT_BUSINESS_TYPE_CASH;
            default: return EMT_BUSINESS_TYPE_UNKNOWN;
        }
    }
    std::string EMTGateway::convert_trade_symbol_to_symbol(data_type::Exchange exchange, std::string trade_symbol)
    {
        switch (exchange)
        {
            case data_type::Exchange::SSE: return std::format("{}.SH", trade_symbol);
            case data_type::Exchange::SZSE: return std::format("{}.SZ", trade_symbol);
            case data_type::Exchange::BSE: return std::format("{}.BJ", trade_symbol);
            default: return trade_symbol;
        }
    }
    util::DateTime EMTGateway::convert_datetime(int64_t time)
    {
        auto millisecond = static_cast<int>(time % 1000);
        time /= 1000;
        auto second = static_cast<int>(time % 100);
        time /= 100;
        auto minute = static_cast<int>(time % 100);
        time /= 100;
        auto hour = static_cast<int>(time % 100);
        time /= 100;
        auto day = static_cast<int>(time % 100);
        time /= 100;
        auto month = static_cast<int>(time % 100);
        time /= 100;
        auto year = static_cast<int>(time);

        return util::DateTime({year, month, day, hour, minute, second, millisecond});
    }
}