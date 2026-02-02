//
//
// Created by root on 2025/10/14.
// 用于测试engine接口功能
//
#include <replxx.hxx>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <magic_enum/magic_enum.hpp>
#include "rk_terminal.hpp"
#include "handle_cmd.hpp"
#include "data_type.h"
#include "interface.h"
#include "engine_impl.h"
#include "util/datetime.h"
#include "util/str.h"

using namespace rk;


int main()
{
    replxx::Replxx rx;
    // 配置终端对话框
    rk::util::init_logger("logs/rk_terminal.log", "rk_terminal");
    rx.install_window_change_handler();
    rk_terminal::TabHintMode mode = rk_terminal::TabHintMode::ChooseConfigFile;
    std::vector<std::string> config_file_paths;
    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(".")))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".toml")
        {
            config_file_paths.emplace_back(entry.path().filename().string());
        }
    }
    const std::vector<std::string> engine_api = {
        "start_trading",
        "stop_trading",
        "query_last_tick",
        "query_symbol_detail",
        "query_all_positions",
        "query_position_data",
        "query_order_trade_cancel_num",
        "order_insert",
        "order_cancel",
        "algo_insert",
        "csv_order_insert",
        "csv_order_cancel",
        "show_table"
    };
    std::unordered_set<data_type::Symbol> symbols;
    rx.set_completion_callback([&](std::string const& input, int& context_len)
   {
       replxx::Replxx::completions_t out;
       size_t pos = input.find_last_of(" \t");
       std::string prefix = (pos == std::string::npos) ? input : input.substr(pos + 1);
       std::vector<std::string> hints;
       switch (mode)
       {
           case rk_terminal::TabHintMode::ChooseConfigFile:
           {
               hints = config_file_paths;
               break;
           }
           case rk_terminal::TabHintMode::ChooseApi:
           {
               hints = engine_api;
               break;
           }
           case rk_terminal::TabHintMode::InputSymbol:
           {
               for (auto symbol : symbols)
               {
                   hints.emplace_back(std::move(symbol.symbol));
               }
               break;
           }
           case rk_terminal::TabHintMode::InputDirection:
           {
               hints.emplace_back(magic_enum::enum_name(data_type::Direction::LONG).data());
               hints.emplace_back(magic_enum::enum_name(data_type::Direction::SHORT).data());
               break;
           }
           case rk_terminal::TabHintMode::InputOffset:
           {
               hints.emplace_back(magic_enum::enum_name(data_type::Offset::OPEN).data());
               hints.emplace_back(magic_enum::enum_name(data_type::Offset::CLOSE).data());
               hints.emplace_back(magic_enum::enum_name(data_type::Offset::CLOSE_TD).data());
               hints.emplace_back(magic_enum::enum_name(data_type::Offset::CLOSE_YD).data());
               break;
           }
           case rk_terminal::TabHintMode::InputAlgoName:
           {
               hints.emplace_back("TWAP");
               break;
           }
           default:
               break;
       }
       for (auto const& hint : hints)
       {
           if (hint.rfind(prefix, 0) == 0)
           {
               out.emplace_back(hint);
           }
       }
       context_len = static_cast<int>(prefix.size());

       return out;

   });
    // 选择配置文件
    const char* c_config_file = rx.input("\x1b[1;32mchoose_config_file\x1b[0m>");
    if (c_config_file == nullptr)
    {
        rx.print("\n(EOF) exit\n");
        return 0;
    }
    // 配置交易engine
    EngineImpl engine{c_config_file};
    auto terminal = std::make_shared<rk_terminal::RKTerminal>(engine, rx, nullptr);
    auto strategy_id = engine.register_strategy(terminal);

    // 主线程处理终端请求
    rk_terminal::handle_cmd(rx, engine, mode, engine_api, symbols, strategy_id);
    return 0;
}