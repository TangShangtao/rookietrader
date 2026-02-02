#pragma once
#include "../adapter.h"
#include <thread>
#include <zmq_addon.hpp>
namespace rk::adapter
{
    class MDGatewayAdapter final : public MDAdapter
    {
    public:
        MDGatewayAdapter(MDAdapter::PushDataCallbacks push_data_callbacks, config_type::MDAdapterConfig config);
        ~MDGatewayAdapter() override = default;
        bool login() override;
        void logout() override;
        bool subscribe(std::unordered_set<data_type::Symbol>) override;
        bool unsubscribe(std::unordered_set<data_type::Symbol>) override;
        std::optional<std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>>> query_symbol_detail() override;
    private:
        void working_loop(const std::stop_token&);
        zmq::socket_t _req_outer;
        zmq::socket_t _req_inner;
        zmq::socket_t _rsp_outer;
        zmq::socket_t _rsp_inner;
        zmq::socket_t _dealer;
        std::unique_ptr<std::jthread> _ipc_worker;

    };

};