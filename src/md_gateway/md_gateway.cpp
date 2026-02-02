#include "md_gateway.h"
#include "util/logger.h"
#include "util/datetime.h"
#include "util/str.h"
#include "util/ipc.h"
#include <zmq_addon.hpp>
namespace rk::gateway
{

    MDGateway::MDGateway(config_type::MDGatewayConfig config)
    :
    _config{std::move(config)},
    _adapter{
        adapter::create_md_adapter(
            {
                [this] (data_type::TickData&& data)
                {
                    _tick_data_queue.enqueue(data);
                },
                [] (){}
            },
            _config.md_adapter_config
        )
    },
    _tick_data_queue{100},
    _busy_worker(
        [this] (const std::stop_token& stop_token) { working_loop(stop_token); }
    )
    {
    }
    bool MDGateway::start_trading()
    {
        RK_LOG_INFO("adapter {} user {} login market...", _config.md_adapter_config.adapter_name, _config.md_adapter_config.user_id.c_str());
        if (!_adapter->login())
        {
            RK_LOG_ERROR("login market failed!");
            return false;
        }
        RK_LOG_INFO("query symbol detail...");
        auto symbol_detail = _adapter->query_symbol_detail();
        if (!symbol_detail)
        {
            RK_LOG_ERROR("query symbol detail failed!");
            return false;
        }
        for (const auto& [symbol, detail] : symbol_detail.value())
        {
            if (
                std::find(
                    _config.md_adapter_config.product_class.begin(),
                    _config.md_adapter_config.product_class.end(),
                    magic_enum::enum_name(symbol.product_class)
                ) == _config.md_adapter_config.product_class.end()
            )
                continue;
            _symbol_detail.emplace(symbol, detail);
        }
        RK_LOG_INFO("{} num of symbol queried, start trading!", _symbol_detail.size());
        return true;
    }
    void MDGateway::stop_trading()
    {
        _adapter->logout();
    }
    std::tuple<ipc::RspData, ipc::SubscribeRsp> MDGateway::handle_subscribe(std::string client_id, const ipc::SubscribeReq& req)
    {
        std::unordered_set<data_type::Symbol> add;
        if (req.symbol_list.empty())
        {
            // 全订阅
            for (const auto& [symbol, _] : _symbol_detail)
            {
                if (!_subscribe_reference.contains(symbol))
                {
                    add.emplace(symbol);
                }
                _subscribe_reference.emplace(symbol, client_id);
            }
        }
        else
        {
            for (const auto& symbol : req.symbol_list)
            {
                if (!_symbol_detail.contains(symbol))
                {
                    RK_LOG_WARN("client id {} subscribe symbol {} not found", util::to_hex_string(client_id), symbol.symbol.to_string());
                    return {{false, ipc::RPCType::SUBSCRIBE}, {}};
                }
                if (!_subscribe_reference.contains(symbol))
                {
                    add.emplace(symbol);
                }
                _subscribe_reference.emplace(symbol, client_id);
            }
        }
        if (!add.empty())
        {
            if (!_adapter->subscribe(add))
            {
                RK_LOG_ERROR("client id {} subscribe failed!", util::to_hex_string(client_id));
                return {{false, ipc::RPCType::SUBSCRIBE}, {}};
            }
        }
        RK_LOG_INFO("client id {} subscribe success, sub num: {} add num: {}", util::to_hex_string(client_id), req.symbol_list.size(), add.size());
        return {{true, ipc::RPCType::SUBSCRIBE}, {}};
    }
    std::tuple<ipc::RspData, ipc::UnsubscribeRsp> MDGateway::handle_unsubscribe(std::string client_id, const ipc::UnsubscribeReq& req)
    {
        std::unordered_set<data_type::Symbol> add;
        if (req.symbol_list.empty())
        {
            // 全退订
            for (const auto& [symbol, _] : _symbol_detail)
            {
                if (!_subscribe_reference.contains(symbol)) continue;
                auto range = _subscribe_reference.equal_range(symbol);
                for (auto it = range.first; it != range.second; )
                {
                    if (it->second == client_id)
                    {
                        _subscribe_reference.erase(it);
                        break;
                    }
                    else ++it;
                }
                if (!_subscribe_reference.contains(symbol)) add.emplace(symbol);
            }
        }
        else
        {
            for (const auto& symbol : req.symbol_list)
            {
                if (!_symbol_detail.contains(symbol))
                {
                    RK_LOG_WARN("client id {} unsubscribe symbol {} not found", util::to_hex_string(client_id), symbol.symbol.to_string());
                    return {{false, ipc::RPCType::UNSUBSCRIBE}, {}};
                }
                if (!_subscribe_reference.contains(symbol)) continue;
                auto range = _subscribe_reference.equal_range(symbol);
                for (auto it = range.first; it != range.second; )
                {
                    if (it->second == client_id)
                    {
                        _subscribe_reference.erase(it);
                        break;
                    }
                    else ++it;
                }
                if (!_subscribe_reference.contains(symbol)) add.emplace(symbol);
            }
        }
        if (!add.empty())
        {
            if (!_adapter->unsubscribe(add))
            {
                RK_LOG_ERROR("client id {} unsubscribe failed!", util::to_hex_string(client_id));
                return {{false, ipc::RPCType::UNSUBSCRIBE}, {}};
            }
        }
        RK_LOG_INFO("client id {} unsubscribe success, unsub num: {} add num: {}", util::to_hex_string(client_id), req.symbol_list.size(), add.size());
        return {{true, ipc::RPCType::UNSUBSCRIBE}, {}};
    }
    std::tuple<ipc::RspData, std::vector<data_type::SymbolDetail>> MDGateway::handle_query_symbol_detail(const std::string& client_id, const ipc::QuerySymbolDetailReq& req)
    {
        std::vector<data_type::SymbolDetail> add;
        add.reserve(_symbol_detail.size());
        if (_symbol_detail.empty())
        {
            RK_LOG_WARN("client id {} symbol detail not ready but query received", util::to_hex_string(client_id));
            return {{false, ipc::RPCType::QUERY_SYMBOL_DETAIL}, {}};
        }
        if (req.symbol_list.empty())
        {
            // 全返回
            for (const auto& [symbol, detail] : _symbol_detail)
            {
                add.emplace_back(*detail);
            }
        }
        else
        {
            for (const auto& symbol : req.symbol_list)
            {
                if (!_symbol_detail.contains(symbol))
                {
                    RK_LOG_WARN("client id {} query symbol {} not found", util::to_hex_string(client_id), symbol.symbol.to_string());
                    return {{false, ipc::RPCType::QUERY_SYMBOL_DETAIL}, {}};
                }
                add.emplace_back(*_symbol_detail[symbol]);
            }
        }
        RK_LOG_INFO("client id {} query symbol detail success, query num: {} add num: {}", util::to_hex_string(client_id), req.symbol_list.size(), add.size());
        return {{true, ipc::RPCType::QUERY_SYMBOL_DETAIL, add.size()}, add};
    }

    void MDGateway::working_loop(const std::stop_token& stop_token)
    {
        auto& context = ipc::get_zmq_context_instance();
        zmq::socket_t router(context, zmq::socket_type::router);
        router.set(zmq::sockopt::router_mandatory, 1);
        router.set(zmq::sockopt::sndhwm, 1000);
        router.bind(_config.endpoint);
        while (!stop_token.stop_requested())
        {
            zmq::pollitem_t items[] = {{router.handle(), 0, ZMQ_POLLIN, 0}};
            zmq::poll(items, 1, std::chrono::milliseconds(1));
            // 处理请求
            if (items[0].revents & ZMQ_POLLIN)
            {
                // 解析请求
                auto res = std::optional<unsigned long>();
                zmq::message_t zmq_client_id_buf{};
                res = router.recv(zmq_client_id_buf, zmq::recv_flags::none);
                std::string zmq_client_id{static_cast<const char*>(zmq_client_id_buf.data()), zmq_client_id_buf.size()};
                if (res == std::nullopt) throw std::runtime_error("zmq::recv return null");
                zmq::message_t ipc_data_header_buf{};
                res = router.recv(ipc_data_header_buf, zmq::recv_flags::none);
                if (res == std::nullopt) throw std::runtime_error("zmq::recv return null");
                const auto req_ipc_data = static_cast<const ipc::IPCData*>(ipc_data_header_buf.data());
                if (req_ipc_data->type != ipc::IPCDataType::REQ)
                {
                    RK_LOG_ERROR("IPCDataType is not req!");
                }
                else
                {
                    zmq::message_t req_header_buf{};
                    res = router.recv(req_header_buf, zmq::recv_flags::none);
                    if (res == std::nullopt) throw std::runtime_error("zmq::recv return null");
                    zmq::message_t req_payload_buf{};
                    res = router.recv(req_payload_buf, zmq::recv_flags::none);
                    if (res == std::nullopt) throw std::runtime_error("zmq::recv return null");
                    auto req_header = static_cast<const ipc::ReqData*>(req_header_buf.data());
                    RK_LOG_DEBUG("req received: client id {} req {}", zmq_client_id, ipc::to_json(*req_header).dump());
                    auto rsp_ipc_data = ipc::IPCData{ipc::IPCDataType::RSP};
                    // TODO 构造span前进行安全性检查
                    switch (req_header->type)
                    {
                        // 处理订阅请求
                        case ipc::RPCType::SUBSCRIBE:
                        {
                            ipc::SubscribeReq req{{static_cast<const data_type::Symbol*>(req_payload_buf.data()), req_header->list_len}};

                            const auto& [rsp_header, rsp_payload] = handle_subscribe(zmq_client_id, req);
                            try
                            {
                                router.send(zmq_client_id_buf, zmq::send_flags::sndmore);
                                router.send(zmq::message_t(&rsp_ipc_data, sizeof(rsp_ipc_data)), zmq::send_flags::sndmore);
                                router.send(zmq::message_t(&rsp_header, sizeof(rsp_header)), zmq::send_flags::sndmore);
                                router.send(zmq::message_t(&rsp_payload, sizeof(rsp_payload)), zmq::send_flags::dontwait);
                            }
                            catch (const zmq::error_t& zmq_error)
                            {
                                RK_LOG_WARN("client id {} error {}", zmq_client_id, zmq_error.what());
                                handle_unsubscribe(zmq_client_id, {});
                            }
                            break;
                        }
                        case ipc::RPCType::UNSUBSCRIBE:
                        {
                            ipc::UnsubscribeReq req{{static_cast<const data_type::Symbol*>(req_payload_buf.data()), req_header->list_len}};
                            const auto& [rsp_header, rsp_payload] = handle_unsubscribe(zmq_client_id, req);
                            try
                            {
                                router.send(zmq_client_id_buf, zmq::send_flags::sndmore);
                                router.send(zmq::message_t{&rsp_ipc_data, sizeof(rsp_ipc_data)}, zmq::send_flags::sndmore);
                                router.send(zmq::message_t(&rsp_header, sizeof(rsp_header)), zmq::send_flags::sndmore);
                                router.send(zmq::message_t(&rsp_payload, sizeof(rsp_payload)), zmq::send_flags::dontwait);
                            }
                            catch (const zmq::error_t& zmq_error)
                            {
                                RK_LOG_WARN("client id {} error {}", zmq_client_id, zmq_error.what());
                                handle_unsubscribe(zmq_client_id, {});
                            }
                            break;
                        }
                        case ipc::RPCType::QUERY_SYMBOL_DETAIL:
                        {
                            ipc::QuerySymbolDetailReq req{{static_cast<const data_type::Symbol*>(req_payload_buf.data()), req_header->list_len}};
                            const auto& [rsp_header, rsp_vec] = handle_query_symbol_detail(zmq_client_id, req);
                            try
                            {
                                auto rsp_payload = std::as_bytes(std::span(rsp_vec));
                                router.send(zmq_client_id_buf, zmq::send_flags::sndmore);
                                router.send(zmq::message_t{&rsp_ipc_data, sizeof(rsp_ipc_data)}, zmq::send_flags::sndmore);
                                router.send(zmq::message_t(&rsp_header, sizeof(rsp_header)), zmq::send_flags::sndmore);
                                router.send(zmq::message_t(rsp_payload.data(), rsp_payload.size_bytes()), zmq::send_flags::dontwait);
                            }
                            catch (const zmq::error_t& zmq_error)
                            {
                                RK_LOG_WARN("client id {} error {}", zmq_client_id, zmq_error.what());
                                handle_unsubscribe(zmq_client_id, {});
                            }
                            break;

                        }
                        default:
                        {
                            break;
                        }
                    }
                }


            }
            // 发布数据
            data_type::TickData data{};
            while (_tick_data_queue.try_dequeue(data))
            {
                if (!_subscribe_reference.contains(data.symbol)) continue;
                auto ipc_data = ipc::IPCData{ipc::IPCDataType::PUB};
                auto pub_data_head = ipc::PubData{ipc::PubDataType::TICK_DATA};
                auto range = _subscribe_reference.equal_range(data.symbol);
                for (auto it = range.first; it != range.second; ++it)
                {
                    const auto& zmq_client_id = it->second;
                    try
                    {
                        router.send(zmq::message_t{zmq_client_id.data(), zmq_client_id.size()}, zmq::send_flags::sndmore);
                        router.send(zmq::message_t{&ipc_data, sizeof(ipc_data)}, zmq::send_flags::sndmore);
                        router.send(zmq::message_t{&pub_data_head, sizeof(pub_data_head)}, zmq::send_flags::sndmore);
                        router.send(zmq::message_t{&data, sizeof(data)}, zmq::send_flags::dontwait);
                    }
                    catch (const zmq::error_t& zmq_error)
                    {
                        RK_LOG_WARN("client id {} error {}", zmq_client_id, zmq_error.what());
                        handle_unsubscribe(zmq_client_id, {});
                        break;
                    }
                }
            }
        }
    }
};