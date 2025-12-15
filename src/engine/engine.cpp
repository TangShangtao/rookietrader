//
// Created by root on 2025/10/20.
//
#include "config_type.h"
#include <vector>
#include <toml++/toml.hpp>

namespace rk::config_type
{
    EngineConfig parse_toml(toml::table config)
    {
        std::vector<std::string> product_class;
        for (const auto& p : *config["gateway_config"]["product_class"].as_array())
        {
            product_class.emplace_back(p.value_or(""));
        }
        return {
            {
                config["account_config"]["account_name"].value_or(""),
            },
            {
                config["log_config"]["log_file_parent_path"].value_or(""),
                config["log_config"]["log_level"].value_or("")
            },
            {
                config["gateway_config"]["gateway_name"].value_or(""),
                product_class,
                config["gateway_config"]["sock_type"].value_or(""),
                config["gateway_config"]["trade_front_ip"].value_or(""),
                config["gateway_config"]["trade_front_port"].value_or(""),
                config["gateway_config"]["market_front_ip"].value_or(""),
                config["gateway_config"]["market_front_port"].value_or(""),
                config["gateway_config"]["broker_id"].value_or(""),
                config["gateway_config"]["user_id"].value_or(""),
                config["gateway_config"]["password"].value_or(""),
                config["gateway_config"]["app_id"].value_or(""),
                config["gateway_config"]["auth_code"].value_or(""),
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
    EngineConfig load_engine_config(std::string_view config_file_path)
    {
        auto config = toml::parse_file(config_file_path);
        return parse_toml(config);

    }
    AlgoExecutorConfig load_algo_executor_config(std::string_view config_file_path)
    {
        auto config = toml::parse_file(config_file_path);
        std::vector<std::tuple<std::string, std::string>> schedule;
        for (auto&& each : *config["schedule"].as_array())
        {
            schedule.emplace_back(
               *each.as_array()[0][0].value<std::string>(),
               *each.as_array()[0][1].value<std::string>()
            );
        }
        return {
            parse_toml(*config["engine_config"].as_table()),
            config["req_endpoint"].as_string()->get(),
            schedule
        };
    }

};
