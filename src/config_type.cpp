//
// Created by root on 2025/10/20.
//
#include "config_type.h"
#include <vector>
#include <toml++/toml.hpp>

namespace rk::config_type
{
    EngineConfig load_engine_config(std::string_view config_file_path)
    {
        auto config = toml::parse_file(config_file_path);
        std::vector<std::string> md_adapter_product_class;
        for (const auto& p : *config["md_adapter_config"]["product_class"].as_array())
        {
            md_adapter_product_class.emplace_back(p.value_or(""));
        }
        std::vector<std::string> td_adapter_product_class;
        for (const auto& p : *config["td_adapter_config"]["product_class"].as_array())
        {
            td_adapter_product_class.emplace_back(p.value_or(""));
        }
        std::vector<std::string> md_adapter_exchange;
        for (const auto& p : *config["md_adapter_config"]["exchange"].as_array())
        {
            md_adapter_exchange.emplace_back(p.value_or(""));
        }
        std::vector<std::string> td_adapter_exchange;
        for (const auto& p : *config["td_adapter_config"]["exchange"].as_array())
        {
            td_adapter_exchange.emplace_back(p.value_or(""));
        }
        return {
            {
                config["account_config"]["account_name"].value_or(""),
            },
            {
                config["md_adapter_config"]["adapter_name"].value_or(""),
                md_adapter_product_class,
                md_adapter_exchange,
                config["md_adapter_config"]["sock_type"].value_or(""),
                config["md_adapter_config"]["trade_front_ip"].value_or(""),
                config["md_adapter_config"]["trade_front_port"].value_or(""),
                config["md_adapter_config"]["market_front_ip"].value_or(""),
                config["md_adapter_config"]["market_front_port"].value_or(""),
                config["md_adapter_config"]["broker_id"].value_or(""),
                config["md_adapter_config"]["user_id"].value_or(""),
                config["md_adapter_config"]["password"].value_or(""),
                config["md_adapter_config"]["app_id"].value_or(""),
                config["md_adapter_config"]["auth_code"].value_or(""),
            },
            {
                config["td_adapter_config"]["adapter_name"].value_or(""),
                td_adapter_product_class,
                td_adapter_exchange,
                config["td_adapter_config"]["sock_type"].value_or(""),
                config["td_adapter_config"]["trade_front_ip"].value_or(""),
                config["td_adapter_config"]["trade_front_port"].value_or(""),
                config["td_adapter_config"]["broker_id"].value_or(""),
                config["td_adapter_config"]["user_id"].value_or(""),
                config["td_adapter_config"]["password"].value_or(""),
                config["td_adapter_config"]["app_id"].value_or(""),
                config["td_adapter_config"]["auth_code"].value_or(""),
            },
            {
                config["risk_control_config"]["daily_order_num"].value_or(0),
                config["risk_control_config"]["daily_cancel_num"].value_or(0),
                config["risk_control_config"]["daily_repeat_order_num"].value_or(0),
            },
            {
                config["db_config"]["user"].value_or(""),
                config["db_config"]["password"].value_or(""),
                config["db_config"]["ip"].value_or(""),
                config["db_config"]["port"].value_or(0),
                config["db_config"]["database"].value_or("")
            }
        };

    }
    AlgoExecutorConfig load_algo_executor_config(std::string_view config_file_path)
    {

        return {};
    }
    MDGatewayConfig MDGatewayConfig::load_config_file(std::string_view config_file_path)
    {
        auto config = toml::parse_file(config_file_path);
        std::vector<std::string> md_adapter_product_class;
        for (const auto& p : *config["md_adapter_config"]["product_class"].as_array())
        {
            md_adapter_product_class.emplace_back(p.value_or(""));
        }
        std::vector<std::string> md_adapter_exchange;
        for (const auto& p : *config["md_adapter_config"]["exchange"].as_array())
        {
            md_adapter_exchange.emplace_back(p.value_or(""));
        }
        return {
            config["endpoint"].value_or(""),
            {
                config["md_adapter_config"]["adapter_name"].value_or(""),
                md_adapter_product_class,
                md_adapter_exchange,
                config["md_adapter_config"]["sock_type"].value_or(""),
                config["md_adapter_config"]["trade_front_ip"].value_or(""),
                config["md_adapter_config"]["trade_front_port"].value_or(""),
                config["md_adapter_config"]["market_front_ip"].value_or(""),
                config["md_adapter_config"]["market_front_port"].value_or(""),
                config["md_adapter_config"]["broker_id"].value_or(""),
                config["md_adapter_config"]["user_id"].value_or(""),
                config["md_adapter_config"]["password"].value_or(""),
                config["md_adapter_config"]["app_id"].value_or(""),
                config["md_adapter_config"]["auth_code"].value_or(""),
            }
        };
    }

};
