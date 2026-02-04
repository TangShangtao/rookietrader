//
// Created by root on 25-9-24.
//

#pragma once
#include <string>
#include <vector>
namespace rk::config_type
{
    struct AccountConfig
    {
        std::string account_name;
    };
    struct MDAdapterConfig
    {
        std::string adapter_name;
        std::vector<std::string> product_class;
        std::vector<std::string> exchange;
        std::string sock_type;
        std::string trade_front_ip;
        std::string trade_front_port;
        std::string market_front_ip;
        std::string market_front_port;
        std::string broker_id;
        std::string user_id;
        std::string password;
        std::string app_id;
        std::string auth_code;
    };
    struct TDAdapterConfig
    {
        std::string adapter_name;
        std::vector<std::string> product_class;
        std::vector<std::string> exchange;
        std::string sock_type;
        std::string trade_front_ip;
        std::string trade_front_port;
        std::string broker_id;
        std::string user_id;
        std::string password;
        std::string app_id;
        std::string auth_code;
    };
    struct RiskControlConfig
    {
        int daily_order_num = 0;
        int daily_cancel_num = 0;
        int daily_repeat_order_num = 0;

    };
    struct DBConfig
    {
        std::string user;
        std::string password;
        std::string ip;
        int port;
        std::string database;
    };
    struct EngineConfig
    {
        AccountConfig account_config;
        MDAdapterConfig md_adapter_config;
        TDAdapterConfig td_adapter_config;
        RiskControlConfig risk_control_config;
        DBConfig db_config;
    };
    EngineConfig load_engine_config(std::string_view config_file_path);
    struct MDGatewayConfig
    {
        std::string endpoint;
        MDAdapterConfig md_adapter_config;
        static MDGatewayConfig load_config_file(std::string_view config_file_path);
    };
    struct AlgoExecutorConfig
    {
        EngineConfig engine_config;
        std::string req_endpoint;
        std::vector<std::tuple<std::string, std::string>> schedule;

    };
    AlgoExecutorConfig load_algo_executor_config(std::string_view config_file_path);
};