#pragma once
#include "data_type.h"
#include "config_type.h"
#include "adapter/adapter.h"
#include <magic_enum/magic_enum.hpp>
#include <thread>
#include <readerwriterqueue.h>
namespace rk::gateway
{
    class MDGateway
    {
    public:
        explicit MDGateway(config_type::MDGatewayConfig config);
        MDGateway() = delete;
        MDGateway(const MDGateway&) = delete;
        MDGateway(MDGateway&&) = delete;
        ~MDGateway() = default;
        bool start_trading();
        void stop_trading();

    private:
        std::tuple<ipc::RspData, ipc::SubscribeRsp> handle_subscribe(std::string client_id, const ipc::SubscribeReq& req);
        std::tuple<ipc::RspData, ipc::UnsubscribeRsp> handle_unsubscribe(std::string client_id, const ipc::UnsubscribeReq& req);
        std::tuple<ipc::RspData, std::vector<data_type::SymbolDetail>> handle_query_symbol_detail(const std::string& client_id, const ipc::QuerySymbolDetailReq& req);
        void working_loop(const std::stop_token&);
    private:
        // 配置
        config_type::MDGatewayConfig _config;
        // 生产者
        std::unique_ptr<adapter::MDAdapter> _adapter;
        moodycamel::ReaderWriterQueue<data_type::TickData> _tick_data_queue;
        // 消费者
        std::jthread _busy_worker;
        std::unordered_map<data_type::Symbol, std::shared_ptr<data_type::SymbolDetail>> _symbol_detail;
        std::unordered_multimap<data_type::Symbol, std::string> _subscribe_reference;



    };
};