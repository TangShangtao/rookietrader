#pragma once
#include "util/str.h"
#include "util.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
namespace rk::rk_terminal
{

    enum class TabHintMode
    {
        ChooseConfigFile,
        ChooseApi,
        InputSymbol,
        InputDirection,
        InputOffset,
        InputAlgoName
    };
    void handle_cmd(
        replxx::Replxx& rx,
        EngineImpl& engine,
        TabHintMode& mode,
        const std::vector<std::string>& engine_api,
        std::unordered_set<data_type::Symbol>& symbols,
        uint32_t strategy_id
    )
    {
        mode = TabHintMode::ChooseApi;
        while (true)
        {
            const char *c_input = rx.input(std::format("\x1b[1;32mhstrader({})\x1b[0m>", engine._config.td_adapter_config.user_id));
            if (c_input == nullptr)
            {
                rx.print("\n(EOF) exit\n");
                return;
            }
            std::string choose_api = util::trim_copy(std::string(c_input));
            if (mode != TabHintMode::ChooseApi)
            {
                rx.print("error TabHintMode!\n");
                continue;
            }
            if (std::find(engine_api.begin(), engine_api.end(), choose_api) == engine_api.end())
            {
                rx.print("unknown api %s\n", choose_api.c_str());
                continue;
            }
            if (choose_api == "start_trading")
            {
                auto res = engine.start_trading();
                if (res)
                {
                    const auto& symbol_detail = engine._market_info->_symbol_details;
                    auto keys = symbol_detail | std::views::transform([](const auto& kv) {return kv.first;});
                    symbols = {keys.begin(), keys.end()};
                }
                auto msg = res ? "success" : "failed";
                rx.print("%s start_trading %s!\n", util::DateTime::now().strftime().c_str(), msg);
            }
            if (choose_api == "stop_trading")
            {
                engine.stop_trading();
            }

            if (choose_api == "query_last_tick")
            {
                mode = TabHintMode::InputSymbol;
                c_input = rx.input("symbol:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                mode = TabHintMode::ChooseApi;
                auto symbol = query_symbol(c_input, symbols);
                if (
                    auto it = engine._market_info->_last_tick_data.find(symbol);
                    it != engine._market_info->_last_tick_data.end()
                    )
                {
                    rx.print("%s\n", data_type::to_json(*(it->second)).dump().c_str());
                }
                else
                {
                    rx.print("%s tick not found or trading not started\n", c_input);
                }
            }
            if (choose_api == "query_all_positions")
            {
                const auto& position_data = engine._trade_info->_position_data;
                auto views = position_data | std::views::transform([](const auto& kv) {return *(kv.second);});
                auto positions = std::vector<data_type::PositionData>{views.begin(), views.end()};
                if (positions.empty())
                {
                    rx.print("position empty or trading not started\n");
                }
                else
                {
                    auto arr = nlohmann::ordered_json::array();
                    for (auto &position: positions)
                    {
                        arr.emplace_back(position);
                    }
                    rx.print("%s\n", arr.dump().c_str());
                }
            }
            if (choose_api == "query_symbol_detail")
            {
                mode = TabHintMode::InputSymbol;
                c_input = rx.input("symbol:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                mode = TabHintMode::ChooseApi;
                auto symbol = query_symbol(c_input, symbols);
                if (
                    auto it = engine._market_info->_symbol_details.find(symbol);
                    it != engine._market_info->_symbol_details.end()
                    )
                {
                    rx.print("%s\n", data_type::to_json(*(it->second)).dump().c_str());
                }
                else
                {
                    rx.print("symbol %s detail no found or trading not started!\n", c_input);
                }
            }
            if (choose_api == "query_position_data")
            {
                mode = TabHintMode::InputSymbol;
                c_input = rx.input("symbol:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                mode = TabHintMode::ChooseApi;
                auto symbol = query_symbol(c_input, symbols);
                if (
                    auto it = engine._trade_info->_position_data.find(symbol);
                    it != engine._trade_info->_position_data.end()
                    )
                {
                    rx.print("%s\n", data_type::to_json(*(it->second)).dump().c_str());
                }
                else
                {
                    rx.print("symbol %s detail no found or trading not started!\n", c_input);
                }
            }
            if (choose_api == "query_order_trade_cancel_num")
            {
                const auto& trade_info = engine._trade_info;
                auto order_num = trade_info->_order_data.size();
                size_t trade_num = 0;
                for (const auto &trade: trade_info->_trade_data)
                {
                    trade_num += trade.size();
                }
                auto cancel_num = static_cast<int>(std::count_if(
                    trade_info->_order_data.begin(),
                    trade_info->_order_data.end(),
                    [](const auto &data) { return data.canceled_volume != 0; }
                ));
                rx.print("%s order num:%i, trade_num:%i, cancel_num:%i\n", util::DateTime::now().strftime().c_str(),
                         order_num, trade_num, cancel_num);
            }
            if (choose_api == "order_insert")
            {
                mode = TabHintMode::InputSymbol;
                c_input = rx.input("symbol:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                auto symbol = query_symbol(c_input, symbols);
                c_input = rx.input("limit_price:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                auto limit_price = std::stod(c_input);
                c_input = rx.input("volume:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                auto volume = std::stoi(c_input);
                mode = TabHintMode::InputDirection;
                c_input = rx.input("direction:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                auto direction = magic_enum::enum_cast<data_type::Direction>(c_input).value();
                mode = TabHintMode::InputOffset;
                c_input = rx.input("offset:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                auto offset = magic_enum::enum_cast<data_type::Offset>(c_input).value();
                mode = TabHintMode::ChooseApi;
                auto order_ref = engine.order_insert(
                    strategy_id,
                    data_type::OrderReq{symbol, limit_price, static_cast<uint32_t>(volume), direction, offset}
                );
                if (order_ref.has_value())
                {
                    rx.print(
                        "%s send order! order_ref: %i\n",
                        util::DateTime::now().strftime().c_str(),order_ref.value()
                    );
                }
                else
                {
                    rx.print("%s send order failed!\n", util::DateTime::now().strftime().c_str());
                }
            }
            if (choose_api == "order_cancel")
            {
                c_input = rx.input("order_ref:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                auto order_ref = std::stoi(c_input);
                mode = TabHintMode::ChooseApi;
                auto success = engine.order_cancel(strategy_id, order_ref);
                if (success)
                {
                    rx.print("%s cancel order! order_ref: %i\n", util::DateTime::now().strftime().c_str(), order_ref);
                }
                else
                {
                    rx.print("%s cancel order failed!\n", util::DateTime::now().strftime().c_str());
                }

            }
            if (choose_api == "algo_insert")
            {

            }
            if (choose_api == "csv_order_insert")
            {
                c_input = rx.input("csv file:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                std::vector<data_type::OrderReq> csv_order_req{};
                std::ifstream file(c_input);
                if (!file.is_open())
                {
                    rx.print("%s file open failed!\n", c_input);
                    break;
                }
                std::string line;
                while (std::getline(file, line))
                {
                    auto fields = split_simple(line);
                    auto symbol_str = fields[0];
                    auto limit_price = std::stod(fields[1]);
                    auto volume = std::stoi(fields[2]);
                    auto direction = magic_enum::enum_cast<data_type::Direction>(fields[3]).value();
                    auto offset = magic_enum::enum_cast<data_type::Offset>(fields[4]).value();
                    auto symbol_it = std::find_if(
                        symbols.begin(),
                        symbols.end(),
                        [&symbol_str](const data_type::Symbol &symbol) { return symbol.symbol == symbol_str; }
                    );
                    auto symbol = data_type::Symbol{symbol_str};
                    if (symbol_it != symbols.end()) {
                        symbol = *symbol_it;
                    }
                    csv_order_req.emplace_back(symbol, limit_price, static_cast<uint32_t>(volume), direction, offset);
                }
                rx.print("%i num of order req detected.\n", csv_order_req.size());
                for (const auto &req: csv_order_req)
                {
                    auto order_ref = engine.order_insert(
                        strategy_id,
                        req
                    );
                    if (order_ref.has_value())
                    {
                        rx.print(
                            "%s send order! order_ref: %i\n",
                            util::DateTime::now().strftime().c_str(),
                            order_ref.value()
                        );
                    }
                    else
                    {
                        rx.print("%s send order failed!\n", util::DateTime::now().strftime().c_str());
                    }
                }
            }
            if (choose_api == "csv_order_cancel")
            {
                c_input = rx.input("csv file:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                std::vector<data_type::OrderRef> csv_cancel_req{};
                std::ifstream file(c_input);
                if (!file.is_open())
                {
                    rx.print("%s file open failed!\n", c_input);
                    break;
                }
                std::string line;
                while (std::getline(file, line))
                {
                    auto fields = split_simple(line);
                    auto order_ref = std::stoi(fields[0]);
                    csv_cancel_req.emplace_back(order_ref);
                }
                rx.print("%i num of order cancel detected.\n", csv_cancel_req.size());
                for (const auto &req: csv_cancel_req)
                {
                    auto pass = engine.order_cancel(
                        strategy_id,
                        req
                    );
                    if (pass)
                    {
                        rx.print("%s cancel order! order_ref: %i\n", util::DateTime::now().strftime().c_str(), req);
                    }
                    else
                    {
                        rx.print("%s cancel order failed! order_ref: %i\n", util::DateTime::now().strftime().c_str(),req);
                    }

                }
            }
            if (choose_api == "show_table")
            {
                auto position_values = (
                    engine._trade_info->_position_data |
                    std::views::values |
                    std::views::filter([](auto& ptr) {return !ptr->empty();}) |
                    std::views::transform([](auto& ptr){return *ptr;})
                );
                auto position = ftxui::Table{to_row(std::vector<data_type::PositionData>{position_values.begin(), position_values.end()})};
                position.SelectAll().Border(ftxui::LIGHT);
                position.SelectAll().SeparatorVertical(ftxui::LIGHT);
                position.SelectAll().SeparatorHorizontal(ftxui::LIGHT);
                position.SelectRow(0).Decorate(ftxui::bold);
                auto order = ftxui::Table{to_row(engine._trade_info->_order_data)};
                order.SelectAll().Border(ftxui::LIGHT);
                order.SelectAll().SeparatorVertical(ftxui::LIGHT);
                order.SelectAll().SeparatorHorizontal(ftxui::LIGHT);
                order.SelectRow(0).Decorate(ftxui::bold);
                auto trade_values = engine._trade_info->_trade_data | std::views::join;
                auto trade = ftxui::Table{to_row(std::vector<data_type::TradeData>{trade_values.begin(), trade_values.end()})};
                trade.SelectAll().Border(ftxui::LIGHT);
                trade.SelectAll().SeparatorVertical(ftxui::LIGHT);
                trade.SelectAll().SeparatorHorizontal(ftxui::LIGHT);
                trade.SelectRow(0).Decorate(ftxui::bold);
                auto account = ftxui::Table{to_row(engine._trade_info->_account_data)};
                account.SelectAll().Border(ftxui::LIGHT);
                account.SelectAll().SeparatorVertical(ftxui::LIGHT);
                account.SelectAll().SeparatorHorizontal(ftxui::LIGHT);
                account.SelectRow(0).Decorate(ftxui::bold);
                constexpr const char* CLEAR_SCREEN = "\x1b[2J\x1b[H";

                auto doc = ftxui::vbox({
                    ftxui::text("account") | ftxui::bold,
                    account.Render(),
                    ftxui::text("order") | ftxui::bold,
                    order.Render(),
                    ftxui::text("trade") | ftxui::bold,
                    trade.Render(),
                    ftxui::text("position") | ftxui::bold,
                    position.Render()
                }) | ftxui::border;
                auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(),ftxui::Dimension::Fit(doc, true));
                Render(screen, doc);
                rx.print("%s%s", CLEAR_SCREEN, screen.ToString().c_str());
            }
        }
    }
};