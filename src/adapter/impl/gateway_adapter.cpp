#include "gateway_adapter.h"
#include "util/ipc.h"
#include "util/logger.h"
#include "util/str.h"
#include <ranges>
namespace rk::adapter
{
    MDGatewayAdapter::MDGatewayAdapter(MDAdapter::PushDataCallbacks push_data_callbacks, config_type::MDAdapterConfig config)
        :
        MDAdapter(std::move(push_data_callbacks), std::move(config)),
        _req_outer(ipc::get_zmq_context_instance(), zmq::socket_type::pair),
        _req_inner(ipc::get_zmq_context_instance(), zmq::socket_type::pair),
        _rsp_outer(ipc::get_zmq_context_instance(), zmq::socket_type::pair),
        _rsp_inner(ipc::get_zmq_context_instance(), zmq::socket_type::pair),
        _dealer(ipc::get_zmq_context_instance(), zmq::socket_type::dealer)
    {
        _req_inner.bind("inproc://req_inner");
        _rsp_inner.bind("inproc://rsp_inner");
        _dealer.connect(std::format("{}://{}:{}", _config.sock_type, _config.market_front_ip, _config.market_front_port));
        RK_LOG_INFO("gateway adapter client id {}", util::to_hex_string(_dealer.get(zmq::sockopt::routing_id)));
        _ipc_worker = std::make_unique<std::jthread>(
            [this] (const std::stop_token& stop_token) { working_loop(stop_token); }
        );

    }
    bool MDGatewayAdapter::login()
    {
        _req_outer.connect("inproc://req_inner");
        _rsp_outer.connect("inproc://rsp_inner");
        return true;
    }
    void MDGatewayAdapter::logout()
    {
        unsubscribe({});
        _req_outer.disconnect("inproc://req_inner");
        _rsp_outer.disconnect("inproc://rsp_inner");
    }
    bool MDGatewayAdapter::subscribe(std::unordered_set<data_type::Symbol> to_sub)
    {
        RK_LOG_INFO("subscribe market data(num: {})...", to_sub.size());
        auto symbol_list = std::vector<data_type::Symbol>{};
        symbol_list.reserve(to_sub.size());
        for (const auto& each : to_sub)
        {
            symbol_list.emplace_back(each);
        }
        auto ipc_header = ipc::IPCData{ipc::IPCDataType::REQ};
        auto req_header = ipc::ReqData{ipc::RPCType::SUBSCRIBE, symbol_list.size()};
        auto req_payload = ipc::SubscribeReq{{symbol_list.data(), symbol_list.size()}};
        auto res = std::optional<size_t>(std::nullopt);
        res = _req_outer.send(zmq::message_t{&ipc_header, sizeof(ipc_header)}, zmq::send_flags::sndmore);
        if (res == std::nullopt)
        {
            RK_LOG_ERROR("subscribe failed! dealer send return null");
            return false;
        }
        res = _req_outer.send(zmq::message_t{&req_header, sizeof(req_header)}, zmq::send_flags::sndmore);
        if (res == std::nullopt)
        {
            RK_LOG_ERROR("subscribe failed! dealer send return null");
            return false;
        }
        res = _req_outer.send(zmq::message_t{req_payload.symbol_list.data(), req_payload.symbol_list.size_bytes()}, zmq::send_flags::dontwait);
        if (res == std::nullopt)
        {
            RK_LOG_ERROR("subscribe failed! dealer send return null");
            return false;
        }
        while (true)
        {
            zmq::pollitem_t items[] = {{_rsp_outer.handle(), 0, ZMQ_POLLIN, 0}};
            zmq::poll(items, 1, std::chrono::milliseconds(10));
            if (!(items[0].revents & ZMQ_POLLIN)) continue;
            zmq::message_t rsp_header_buf{};
            res = _rsp_outer.recv(rsp_header_buf, zmq::recv_flags::none);
            if (res == std::nullopt)
            {
                RK_LOG_ERROR("subscribe failed! dealer recv return null");
                return false;
            }
            zmq::message_t rsp_payload_buf{};
            res = _rsp_outer.recv(rsp_payload_buf, zmq::recv_flags::none);
            if (res == std::nullopt)
            {
                RK_LOG_ERROR("subscribe failed! dealer recv return null");
                return false;
            }
            auto rsp_header = static_cast<const ipc::RspData*>(rsp_header_buf.data());
            if (!rsp_header->res)
            {
                RK_LOG_ERROR("subscribe failed! rsp res false");
                return false;
            }
            RK_LOG_INFO("subscribe success!");
            return true;
        }

    }
    bool MDGatewayAdapter::unsubscribe(std::unordered_set<data_type::Symbol> to_unsub)
    {
        RK_LOG_INFO("unsubscribe market data(num: {})...", to_unsub.size());
        auto symbol_list = std::vector<data_type::Symbol>{to_unsub.size()};
        for (const auto& each : to_unsub)
        {
            symbol_list.emplace_back(each);
        }
        auto ipc_header = ipc::IPCData{ipc::IPCDataType::REQ};
        auto req_header = ipc::ReqData{ipc::RPCType::UNSUBSCRIBE, symbol_list.size()};
        auto req_payload = ipc::UnsubscribeReq{{symbol_list.data(), symbol_list.size()}};
        auto res = std::optional<size_t>(std::nullopt);
        res = _req_outer.send(zmq::message_t{&ipc_header, sizeof(ipc_header)}, zmq::send_flags::sndmore);
        if (res == std::nullopt)
        {
            RK_LOG_ERROR("unsubscribe failed! dealer send return null");
            return false;
        }
        res = _req_outer.send(zmq::message_t{&req_header, sizeof(req_header)}, zmq::send_flags::sndmore);
        if (res == std::nullopt)
        {
            RK_LOG_ERROR("unsubscribe failed! dealer send return null");
            return false;
        }
        res = _req_outer.send(zmq::message_t{req_payload.symbol_list.data(), req_payload.symbol_list.size_bytes()}, zmq::send_flags::dontwait);
        if (res == std::nullopt)
        {
            RK_LOG_ERROR("unsubscribe failed! dealer send return null");
            return false;
        }
        while (true)
        {
            zmq::pollitem_t items[] = {{_rsp_outer.handle(), 0, ZMQ_POLLIN, 0}};
            zmq::poll(items, 1, std::chrono::milliseconds(10));
            if (!(items[0].revents & ZMQ_POLLIN)) continue;
            zmq::message_t rsp_header_buf{};
            res = _rsp_outer.recv(rsp_header_buf, zmq::recv_flags::none);
            if (res == std::nullopt)
            {
                RK_LOG_ERROR("unsubscribe failed! dealer recv return null");
                return false;
            }
            zmq::message_t rsp_payload_buf{};
            res = _rsp_outer.recv(rsp_payload_buf, zmq::recv_flags::none);
            if (res == std::nullopt)
            {
                RK_LOG_ERROR("unsubscribe failed! dealer recv return null");
                return false;
            }
            auto rsp_header = static_cast<const ipc::RspData*>(rsp_header_buf.data());
            if (!rsp_header->res)
            {
                RK_LOG_ERROR("unsubscribe failed! rsp res false");
                return false;
            }
            RK_LOG_INFO("unsubscribe succeed!");
            return true;
        }
    }

    std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>>> MDGatewayAdapter::query_symbol_detail()
    {
        RK_LOG_INFO("query symbol detail...");
        auto ipc_header = ipc::IPCData{ipc::IPCDataType::REQ};
        auto req_header = ipc::ReqData{ipc::RPCType::QUERY_SYMBOL_DETAIL};
        auto req_payload = ipc::QuerySymbolDetailReq{};
        auto res = std::optional<size_t>(std::nullopt);
        res = _req_outer.send(zmq::message_t{&ipc_header, sizeof(ipc_header)}, zmq::send_flags::sndmore);
        if (res == std::nullopt)
        {
            RK_LOG_ERROR("query symbol detail failed! dealer send return null");
            return std::nullopt;
        }
        res = _req_outer.send(zmq::message_t{&req_header, sizeof(req_header)}, zmq::send_flags::sndmore);
        if (res == std::nullopt)
        {
            RK_LOG_ERROR("query symbol detail failed! dealer send return null");
            return std::nullopt;
        }
        res = _req_outer.send(zmq::message_t{req_payload.symbol_list.data(), req_payload.symbol_list.size_bytes()}, zmq::send_flags::dontwait);
        if (res == std::nullopt)
        {
            RK_LOG_ERROR("query symbol detail failed! dealer send return null");
            return std::nullopt;
        }
        while (true)
        {
            zmq::pollitem_t items[] = {{_rsp_outer.handle(), 0, ZMQ_POLLIN, 0}};
            zmq::poll(items, 1, std::chrono::milliseconds(10));
            if (!(items[0].revents & ZMQ_POLLIN)) continue;
            zmq::message_t rsp_header_buf{};
            res = _rsp_outer.recv(rsp_header_buf, zmq::recv_flags::none);
            if (res == std::nullopt)
            {
                RK_LOG_ERROR("query symbol detail failed! dealer recv return null");
                return std::nullopt;
            }
            zmq::message_t rsp_payload_buf{};
            res = _rsp_outer.recv(rsp_payload_buf, zmq::recv_flags::none);
            if (res == std::nullopt)
            {
                RK_LOG_ERROR("query symbol detail failed! dealer recv return null");
                return std::nullopt;
            }
            auto rsp_header = static_cast<const ipc::RspData*>(rsp_header_buf.data());
            if (!rsp_header->res)
            {
                RK_LOG_ERROR("query symbol detail failed! rsp res false");
                return std::nullopt;
            }
            auto rsp_payload = ipc::QuerySymbolDetailRsp{{static_cast<const data_type::SymbolDetail*>(rsp_payload_buf.data()), rsp_header->list_len}};
            RK_LOG_INFO("query symbol detail success! num {}", rsp_header->list_len);
            std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>> symbol_detail;
            for (const auto& each : rsp_payload.symbol_detail_list)
            {
                if (std::ranges::find(_config.exchange, magic_enum::enum_name(each.symbol.exchange)) == _config.exchange.end())
                {
                    continue;
                }
                symbol_detail.emplace(each.symbol, std::make_shared<data_type::SymbolDetail>(each));
            }
            return symbol_detail;
        }
    }
    void MDGatewayAdapter::working_loop(const std::stop_token& stop_token)
    {
        while (!stop_token.stop_requested())
        {
            auto res = std::optional<size_t>(std::nullopt);
            zmq::pollitem_t items[] = {
                {_req_inner.handle(), 0, ZMQ_POLLIN, 0},
                {_dealer.handle(), 0, ZMQ_POLLIN, 0}
            };
            zmq::poll(items, 2, std::chrono::milliseconds(10));
            if (items[0].revents & ZMQ_POLLIN)
            {
                std::vector<zmq::message_t> msg;
                msg.reserve(3);
                res = zmq::recv_multipart(_req_inner, std::back_inserter(msg));
                if (res == std::nullopt)
                {
                    RK_LOG_ERROR("_req_inner recv null");
                    return;
                }
                res = zmq::send_multipart(_dealer, msg, zmq::send_flags::none);
                if (res == std::nullopt)
                {
                    RK_LOG_ERROR("dealer recv null");
                    return;
                }
            }
            if (items[1].revents & ZMQ_POLLIN)
            {
                std::vector<zmq::message_t> msg;
                msg.reserve(5);
                res = zmq::recv_multipart(_dealer, std::back_inserter(msg));
                if (res == std::nullopt)
                {
                    RK_LOG_ERROR("dealer recv null");
                    return;
                }
                auto ipc_data = static_cast<const ipc::IPCData*>(msg[0].data());
                switch (ipc_data->type)
                {
                    case ipc::IPCDataType::RSP:
                    {
                        _rsp_inner.send(msg[1], zmq::send_flags::sndmore);
                        _rsp_inner.send(msg[2], zmq::send_flags::dontwait);
                        break;
                    }
                    case ipc::IPCDataType::PUB:
                    {
                        auto pub_data = static_cast<const ipc::PubData*>(msg[1].data());
                        if (pub_data->type != ipc::PubDataType::TICK_DATA) continue;
                        auto tick_data = data_type::TickData{};
                        std::memcpy(&tick_data, static_cast<const data_type::TickData*>(msg[2].data()), sizeof(tick_data));
                        _push_data_callbacks.push_tick(std::move(tick_data));
                        break;
                    }
                    default:
                    {
                        RK_LOG_ERROR("{}", ipc::to_json(*ipc_data).dump());
                    }
                }
            }
        }
    }
};