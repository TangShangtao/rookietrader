//
//
// Created by root on 2025/10/14.
// 用于测试engine接口功能
//
#include <sstream>
#include <filesystem>
#include <replxx.hxx>
#include <functional>
#include <mutex>
#include <fstream>
#include <magic_enum/magic_enum.hpp>
#include <VariadicTable.h>
#include <crow.h>

#include "data_type.h"
#include "serialization.h"
#include "interface.h"
#include "engine_impl.h"
#include "config_type.h"
#include "util/datetime.h"
#include "util/str.h"


using namespace rk;
class RKTerminal : public interface::Strategy
{
public:
    RKTerminal(
        EngineImpl& engine,
        replxx::Replxx& rx,
        std::function<void(const std::string&)> publish_cb
    )
    : _engine(engine), _rx(rx), _publish_cb(std::move(publish_cb)) {}
    std::unordered_set<data_type::Symbol> on_init(uint32_t trading_day) override
    {
        return _engine.query_all_symbols();
    }
    void on_tick(const data_type::TickData& data) override
    {

    }
    void on_bar(const data_type::BarData& data) override {};
    void on_trade(const data_type::TradeData& data) override
    {
        VariadicTable<std::string, data_type::OrderRef, std::string, double, uint32_t, std::string, double> vt{
            {"callback", "order_ref", "trade_id", "trade_price", "trade_volume", "trade_time", "fee"}
        };
        vt.addRow(
            __FUNCTION__,
            data.order_ref, data.trade_id.to_string(), data.trade_price, data.trade_volume,
            data.trade_time.strftime(),
            data.fee
        );
        std::ostringstream oss;
        vt.print(oss);
        _rx.print("%s\n%s\n", util::DateTime::now().strftime().c_str(), oss.str().c_str());
    }
    void on_cancel(const data_type::CancelData& data) override
    {
        VariadicTable<std::string, data_type::OrderRef, uint32_t, std::string> vt{
            {"callback", "order_ref", "cancel_volume", "cancel_time"}
        };
        vt.addRow(__FUNCTION__, data.order_ref, data.cancel_volume, data.cancel_time.strftime());
        std::ostringstream oss;
        vt.print(oss);
        _rx.print("%s\n%s\n", util::DateTime::now().strftime().c_str(), oss.str().c_str());
    }
    void on_error(const data_type::OrderError& data) override
    {
        VariadicTable<std::string, data_type::OrderRef, std::string, std::string> vt{
            {"callback", "order_ref", "error_type", "error_msg"}
        };
        vt.addRow(__FUNCTION__, data.order_ref, magic_enum::enum_name(data.error_type).data(), data.error_msg.to_string());
        std::ostringstream oss;
        vt.print(oss);
        _rx.print("%s\n%s\n", util::DateTime::now().strftime().c_str(), oss.str().c_str());
    }
private:
    EngineImpl& _engine;
    replxx::Replxx& _rx;
    std::function<void(const std::string&)> _publish_cb;
};
std::vector<std::string> split_simple(const std::string& line, char delim = ',')
{
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, delim)) {
        fields.push_back(item);
    }
    return fields;
}
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
        const char* c_input = rx.input("\x1b[1;32mhstrader\x1b[0m>");
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
        if (
            auto it = std::find(engine_api.begin(), engine_api.end(), choose_api);
            it != engine_api.end()
            )
        {
            if (choose_api == "start_trading")
            {
                auto res = engine.start_trading();
                if (res) symbols = engine.query_all_symbols();
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
                auto symbol_str = std::string(c_input);
                mode = TabHintMode::ChooseApi;
                auto symbol_it = std::find_if(
                    symbols.begin(),
                    symbols.end(),
                    [&symbol_str](const data_type::Symbol& symbol) {return symbol.symbol == symbol_str;}
                );
                auto symbol = data_type::Symbol{symbol_str};
                if (symbol_it != symbols.end())
                {
                    symbol = *symbol_it;
                }
                auto tick = engine.query_last_tick(symbol);
                if (tick == nullptr)
                {
                    rx.print("%s tick not found or trading not started\n", symbol_str.c_str());
                }
                else
                {
                    rx.print("%s\n", data_type::to_json(*tick).dump(4).c_str());
                }


            }
            if (choose_api == "query_all_positions")
            {
                auto positions = engine.query_all_positions();
                if (positions.empty())
                {
                    rx.print("position empty or trading not started\n");
                }
                else
                {
                    auto arr = nlohmann::ordered_json::array();
                    for (auto& position : positions)
                    {
                        arr.emplace_back(std::move(position));
                    }
                    rx.print("%s\n", arr.dump(4).c_str());
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
                auto symbol_str = std::string(c_input);
                auto symbol_it = std::find_if(
                    symbols.begin(),
                    symbols.end(),
                    [&symbol_str](const data_type::Symbol& symbol) {return symbol.symbol == symbol_str;}
                );
                auto symbol = data_type::Symbol{symbol_str};
                if (symbol_it != symbols.end())
                {
                    symbol = *symbol_it;
                }
                mode = TabHintMode::ChooseApi;
                auto symbol_detail = engine.query_symbol_detail(symbol);
                if (symbol_detail == nullptr)
                {
                    rx.print("symbol %s detail no found or trading not started!\n", symbol_str.c_str());
                }
                else
                {
                    rx.print("%s\n", data_type::to_json(*symbol_detail).dump(4).c_str());
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
                auto symbol_str = std::string(c_input);
                auto symbol_it = std::find_if(
                    symbols.begin(),
                    symbols.end(),
                    [&symbol_str](const data_type::Symbol& symbol) {return symbol.symbol == symbol_str;}
                );
                auto symbol = data_type::Symbol{symbol_str};
                if (symbol_it != symbols.end())
                {
                    symbol = *symbol_it;
                }
                mode = TabHintMode::ChooseApi;
                auto position_data = engine.query_position_data(symbol);
                if (position_data == nullptr)
                {
                    rx.print("symbol %s detail no found or trading not started!\n", symbol_str.c_str());
                }
                else
                {
                    rx.print("%s\n", data_type::to_json(*position_data).dump(4).c_str());
                }
            }
            if (choose_api == "query_order_trade_cancel_num")
            {
                auto trade_info = engine.query_trade_info();
                auto order_num = trade_info->_order_data.size();
                auto trade_num = 0;
                for (const auto& trade : trade_info->_trade_data)
                {
                    trade_num += trade.size();
                }
                auto cancel_num = static_cast<int>(std::count_if(
                    trade_info->_order_data.begin(),
                    trade_info->_order_data.end(),
                    [](const auto& data){return data.canceled_volume != 0;}
                ));
                rx.print("%s order num:%i, trade_num:%i, cancel_num:%i\n", util::DateTime::now().strftime().c_str(), order_num, trade_num, cancel_num);
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
                auto symbol_str = std::string(c_input);
                auto limit_price = std::stod(rx.input("limit_price:"));
                auto volume = std::stoi(rx.input("volume:"));
                mode = TabHintMode::InputDirection;
                auto direction = magic_enum::enum_cast<data_type::Direction>(rx.input("direction:")).value();
                mode = TabHintMode::InputOffset;
                auto offset = magic_enum::enum_cast<data_type::Offset>(rx.input("offset:")).value();
                mode = TabHintMode::ChooseApi;
                auto symbol_it = std::find_if(
                    symbols.begin(),
                    symbols.end(),
                    [&symbol_str](const data_type::Symbol& symbol) {return symbol.symbol == symbol_str;}
                );
                auto symbol = data_type::Symbol{symbol_str};
                if (symbol_it != symbols.end())
                {
                    symbol = *symbol_it;
                }
                auto order_ref = engine.order_insert(
                    strategy_id,
                    data_type::OrderReq{symbol, limit_price, static_cast<uint32_t>(volume), direction, offset}
                );
                if (order_ref.has_value()) rx.print("%s send order! order_ref: %i\n", util::DateTime::now().strftime().c_str(), order_ref.value());
                else rx.print("%s send order failed!\n", util::DateTime::now().strftime().c_str());
            }
            if (choose_api == "order_cancel")
            {
                auto order_ref = std::stoi(rx.input("order_ref:"));
                mode = TabHintMode::ChooseApi;
                auto success = engine.order_cancel(strategy_id, order_ref);
                if (success) rx.print("%s cancel order! order_ref: %i\n", util::DateTime::now().strftime().c_str(), order_ref);
                else rx.print("%s cancel order failed!\n", util::DateTime::now().strftime().c_str());

            }
            if (choose_api == "algo_insert")
            {
                mode = TabHintMode::InputSymbol;
                c_input = rx.input("symbol:");
                if (c_input == nullptr)
                {
                    rx.print("\n(EOF) exit\n");
                    return;
                }
                auto symbol_str = std::string(c_input);
                auto net_position = std::stoi(rx.input("net_position:"));
                mode = TabHintMode::InputAlgoName;
                auto algo_name = rx.input("algo_name:");
                auto start_time = rx.input("start_time(%Y-%m-%d %H:%M:%S):");
                auto end_time = rx.input("end_time(%Y-%m-%d %H:%M:%S):");
                mode = TabHintMode::ChooseApi;
                auto symbol = *std::find_if(
                    symbols.begin(),
                    symbols.end(),
                    [&symbol_str](const data_type::Symbol& symbol) {return symbol.symbol == symbol_str;}
                );
                engine.algo_insert(
                    data_type::AlgoReq{
                        symbol, net_position, algo_name, "",
                        util::DateTime::strptime(start_time),
                        util::DateTime::strptime(end_time),
                    }
                );
                rx.print("algo insert!\n");
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
                        [&symbol_str](const data_type::Symbol& symbol) {return symbol.symbol == symbol_str;}
                    );
                    auto symbol = data_type::Symbol{symbol_str};
                    if (symbol_it != symbols.end())
                    {
                        symbol = *symbol_it;
                    }
                    csv_order_req.emplace_back(symbol, limit_price, static_cast<uint32_t>(volume), direction, offset);
                }
                rx.print("%i num of order req detected.\n", csv_order_req.size());
                for (const auto& req : csv_order_req)
                {
                    auto order_ref = engine.order_insert(
                        strategy_id,
                        req
                    );
                    if (order_ref.has_value()) rx.print("%s send order! order_ref: %i\n", util::DateTime::now().strftime().c_str(), order_ref.value());
                    else rx.print("%s send order failed!\n", util::DateTime::now().strftime().c_str());
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
                for (const auto& req : csv_cancel_req)
                {
                    auto pass = engine.order_cancel(
                        strategy_id,
                        req
                    );
                    if (pass) rx.print("%s cancel order! order_ref: %i\n", util::DateTime::now().strftime().c_str(), req);
                    else rx.print("%s cancel order failed! order_ref: %i\n", util::DateTime::now().strftime().c_str(), req);
                }
            }
        }
        else if (choose_api.empty()) {}
        else
        {
            rx.print("unknown api %s\n", choose_api.c_str());
        }
    }
}


int main()
{
    replxx::Replxx rx;
    // 调试
//    EngineImpl _engine{"ctp_simnow_tst.toml"};
//    auto _strategy_id = _engine.register_strategy(std::make_shared<RKTerminal>(_engine, rx, [](const std::string&){}));
//    _engine.order_insert(_strategy_id, data_type::OrderReq{{"rb2601"},1,1,data_type::Direction::LONG, data_type::Offset::OPEN});
//    auto res = _engine.start_trading();
//    if (!res) return -1;
//    auto detail = _engine.engine_detail_json();
//    std::ofstream outfile("output.txt");
//    outfile << detail << std::endl;
//    outfile.close();
//    _engine.stop_trading();
//    rx.input("debugging...");
    // 配置终端对话框
    rx.install_window_change_handler();
    TabHintMode mode = TabHintMode::ChooseConfigFile;
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
        "csv_order_cancel"
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
           case TabHintMode::ChooseConfigFile:
           {
               hints = config_file_paths;
               break;
           }
           case TabHintMode::ChooseApi:
           {
               hints = engine_api;
               break;
           }
           case TabHintMode::InputSymbol:
           {
               for (auto symbol : symbols)
               {
                   hints.emplace_back(std::move(symbol.symbol));
               }
               break;
           }
           case TabHintMode::InputDirection:
           {
               hints.emplace_back(magic_enum::enum_name(data_type::Direction::LONG).data());
               hints.emplace_back(magic_enum::enum_name(data_type::Direction::SHORT).data());
               break;
           }
           case TabHintMode::InputOffset:
           {
               hints.emplace_back(magic_enum::enum_name(data_type::Offset::OPEN).data());
               hints.emplace_back(magic_enum::enum_name(data_type::Offset::CLOSE).data());
               hints.emplace_back(magic_enum::enum_name(data_type::Offset::CLOSE_TD).data());
               hints.emplace_back(magic_enum::enum_name(data_type::Offset::CLOSE_YD).data());
               break;
           }
           case TabHintMode::InputAlgoName:
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
    // 配置web服务
    std::mutex ws_connection_mutex;
    std::unordered_set<crow::websocket::connection*> ws_connections;
    auto publish_cb = [&ws_connection_mutex, &ws_connections](const std::string& json)
    {
        std::lock_guard<std::mutex> lock(ws_connection_mutex);
        auto it = ws_connections.begin();
        while (it != ws_connections.end())
        {
            try
            {
                (*it)->send_text(json);
                ++it;
            }
            catch (...)
            {
                it = ws_connections.erase(it);
            }
        }
    };
    crow::SimpleApp app;
    app.loglevel(crow::LogLevel::Critical);
    CROW_ROUTE(app, "/")([]()
    {
        crow::mustache::context ctx;
        return crow::mustache::load("rk_terminal/index.html").render(ctx);
    });
    CROW_ROUTE(app, "/ws")
        .websocket(&app)
        .onopen([&ws_connection_mutex, &ws_connections](crow::websocket::connection& conn)
        {
            std::lock_guard<std::mutex> lock(ws_connection_mutex);
            ws_connections.insert(&conn);
        })
        .onclose([&ws_connection_mutex, &ws_connections](crow::websocket::connection& conn, const std::string& reason, uint16_t /*code*/)
         {
             std::lock_guard<std::mutex> lock(ws_connection_mutex);
             ws_connections.erase(&conn);
         });
    std::thread server_thread([&app]()
    {
        app.port(18080).multithreaded().run();
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
    auto terminal = std::make_shared<RKTerminal>(engine, rx, std::move(publish_cb));
    auto strategy_id = engine.register_strategy(terminal);

    // 主线程处理终端请求
    handle_cmd(rx, engine, mode, engine_api, symbols, strategy_id);

    app.stop();
    server_thread.join();
    return 0;
}