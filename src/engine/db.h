//
// Created by root on 2025/11/14.
//

#pragma once
#include <thread>
#include <string_view>
#include <pqxx/pqxx>
#include <readerwriterqueue.h>
#include <NanoLogCpp17.h>
#include "config_type.h"
namespace rk::db
{
    namespace table
    {
        struct logs
        {
            static constexpr std::string_view table_name = "rookietrader_logs";
            static constexpr std::string_view create_table = R"(
                CREATE TABLE IF NOT EXISTS rookietrader_logs (
                    trading_day DATE NOT NULL,
                    account_name TEXT NOT NULL,
                    symbol TEXT NULL,
                    trade_symbol TEXT NULL,
                    exchange TEXT NULL,
                    product_class TEXT NULL,
                    order_ref INT NULL,
                    event_type TEXT NOT NULL,
                    logs TEXT NOT NULL,
                    log_level TEXT NOT NULL,
                    action_time TIME NOT NULL,
                    insert_time TIME NOT NULL
                );
                CREATE INDEX IF NOT EXISTS rookietrader_logs_idx ON rookietrader_logs USING btree (trading_day, account_name, symbol);
            )";
            static constexpr std::string_view insert = R"(
                INSERT INTO rookietrader_logs
                (trading_day,account_name,symbol,trade_symbol,exchange,product_class,order_ref,event_type,logs,log_level,action_time,insert_time) VALUES
                ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12);
            )";
        };
        struct orders
        {
            static constexpr std::string_view table_name = "rookietrader_orders";
            static constexpr std::string_view create_table = R"(
                CREATE TABLE IF NOT EXISTS rookietrader_orders (
                    trading_day DATE NOT NULL,
                    account_name TEXT NOT NULL,
                    order_ref INT NULL,
                    symbol TEXT NOT NULL,
                    trade_symbol TEXT NOT NULL,
                    exchange TEXT NOT NULL,
                    product_class TEXT NOT NULL,
                    limit_price FLOAT NOT NULL,
                    volume INT NOT NULL,
                    direction TEXT NOT NULL,
                    "offset" TEXT NOT NULL,
                    req_time TIME NOT NULL,
                    traded_volume INT NOT NULL,
                    remain_volume INT NOT NULL,
                    canceled_volume INT NOT NULL,
                    insert_time TIME NOT NULL
                );
                CREATE INDEX IF NOT EXISTS rookietrader_orders_idx ON rookietrader_orders USING btree (trading_day, account_name, symbol);
            )";
            static constexpr std::string_view insert = R"(
                INSERT INTO rookietrader_orders
                (trading_day,account_name,order_ref,symbol,trade_symbol,exchange,product_class,limit_price,volume,direction,"offset",req_time,traded_volume,remain_volume,canceled_volume,insert_time) VALUES
                ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16);
            )";
        };
        struct trades
        {
            static constexpr std::string_view table_name = "rookietrader_trades";
            static constexpr std::string_view create_table = R"(
                CREATE TABLE IF NOT EXISTS rookietrader_trades (
                    trading_day DATE NOT NULL,
                    account_name TEXT NOT NULL,
                    order_ref INT NULL,
                    symbol TEXT NOT NULL,
                    trade_symbol TEXT NOT NULL,
                    exchange TEXT NOT NULL,
                    product_class TEXT NOT NULL,
                    limit_price FLOAT NOT NULL,
                    volume INT NOT NULL,
                    direction TEXT NOT NULL,
                    "offset" TEXT NOT NULL,
                    trade_id TEXT NOT NULL,
                    trade_price FLOAT NOT NULL,
                    trade_volume INT NOT NULL,
                    trade_time TIME NOT NULL,
                    fee FLOAT NOT NULL,
                    insert_time TIME NOT NULL
                );
                CREATE INDEX IF NOT EXISTS rookietrader_trades_idx ON rookietrader_trades USING btree (trading_day, account_name, symbol);
            )";
            static constexpr std::string_view insert = R"(
                INSERT INTO rookietrader_trades
                (trading_day,account_name,order_ref,symbol,trade_symbol,exchange,product_class,limit_price,volume,direction,"offset",trade_id,trade_price,trade_volume,trade_time,fee,insert_time) VALUES
                ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17);
            )";
        };
        struct positions
        {
            static constexpr std::string_view table_name = "rookietrader_positions";
            static constexpr std::string_view create_table = R"(
                CREATE TABLE IF NOT EXISTS rookietrader_positions (
                    trading_day DATE NOT NULL,
                    account_name TEXT NOT NULL,
                    symbol TEXT NOT NULL,
                    trade_symbol TEXT NOT NULL,
                    exchange TEXT NOT NULL,
                    product_class TEXT NOT NULL,
                    long_position INT NOT NULL,
                    short_position INT NOT NULL,
                    insert_time TIME NOT NULL
                );
                CREATE INDEX IF NOT EXISTS rookietrader_positions_idx ON rookietrader_positions USING btree (trading_day, account_name);
            )";
            static constexpr std::string_view insert = R"(
                INSERT INTO rookietrader_positions
                (trading_day,account_name,symbol,trade_symbol,exchange,product_class,long_position,short_position,insert_time) VALUES
                ($1,$2,$3,$4,$5,$6,$7,$8,$9);
            )";
        };
        struct risk_indicators
        {
            static constexpr std::string_view table_name = "rookietrader_risk_indicators";
            static constexpr std::string_view create_table = R"(
                CREATE TABLE IF NOT EXISTS rookietrader_risk_indicators (
                    trading_day DATE NOT NULL,
                    account_name TEXT NOT NULL,
                    daily_order_num INT NOT NULL,
                    daily_cancel_num INT NOT NULL,
                    daily_repeat_order_num INT NOT NULL,
                    insert_time TIME NOT NULL
                );
                CREATE UNIQUE INDEX IF NOT EXISTS rookietrader_risk_indicators_idx ON rookietrader_risk_indicators USING btree (trading_day, account_name);
            )";
            static constexpr std::string_view insert = R"(
                INSERT INTO rookietrader_risk_indicators
                (trading_day,account_name,daily_order_num,daily_cancel_num,daily_repeat_order_num,insert_time) VALUES
                ($1,$2,$3,$4,$5,$6)
                ON CONFLICT (trading_day, account_name) DO UPDATE SET
                trading_day = EXCLUDED.trading_day, account_name = EXCLUDED.account_name, daily_order_num = EXCLUDED.daily_order_num, daily_cancel_num = EXCLUDED.daily_cancel_num, daily_repeat_order_num = EXCLUDED.daily_repeat_order_num, insert_time = EXCLUDED.insert_time
            )";
        };
    };

    class Executor
    {
    public:
        explicit Executor(const config_type::DBConfig& db_config)
        :
        _async_engine(std::format(
            "host={} port={} dbname={} user={} password={}",
            db_config.ip, db_config.port, db_config.database, db_config.user, db_config.password
        )),
        _sync_engine(std::format(
            "host={} port={} dbname={} user={} password={}",
            db_config.ip, db_config.port, db_config.database, db_config.user, db_config.password
        ))
        {
            create_table_if_not_exists();
            prepare_stmt();

            _busy_worker = std::make_unique<std::jthread>([this](std::stop_token st) -> void
            {
                NanoLog::preallocate();
                std::tuple<std::function<void()>, std::function<void(bool)>> sql_task;
                while (!st.stop_requested())
                {
                    if (_spsc_queue.try_dequeue(sql_task))
                    {
                        bool success = false;
                        auto& [task, callback] = sql_task;
                        try
                        {
                            task();
                            success = true;
                        }
                        catch (const std::exception& e)
                        {
                            std::cout << e.what();
                            NANO_LOG(WARNING, "sql exec error: %s", e.what());
                        }
                        if (callback) callback(success);
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::microseconds(100));
                    }
                }
            });
        }
        bool exec_sync(std::string_view sql)
        {
            bool success = false;
            try
            {
                pqxx::work tx(_sync_engine);
                tx.exec(sql);
                tx.commit();
                success = true;
            }
            catch (const std::exception& e)
            {
                NANO_LOG(WARNING, "sql exec error: %s", e.what());
            }
            return success;
        }
        template<typename... Args>
        bool insert_sync(std::string_view table_name, Args&&... args)
        {
            bool success = false;
            try
            {
                pqxx::work tx(_sync_engine);
                tx.exec(pqxx::prepped{std::format("insert_{}", table_name)}, pqxx::params{std::forward<Args>(args)...});
                tx.commit();
                success = true;
            }
            catch (const std::exception& e)
            {
                NANO_LOG(WARNING, "sql exec error: %s", e.what());
            }
            return success;
        }
        template<typename... Args>
        bool insert_async(std::string_view table_name, std::function<void(bool)> callback, Args&&... args)
        {
            auto task = [
                this,
                name = std::string(table_name),
                args_tuple = std::make_tuple(std::forward<Args>(args)...)
            ]() mutable
            {
                pqxx::work tx(_async_engine);
                std::apply([&](auto&&... unpacked_args)
                    {
                        tx.exec(
                            pqxx::prepped{std::format("insert_{}", name)},
                            pqxx::params{std::forward<decltype(unpacked_args)>(unpacked_args)...}
                        );
                    },
                    args_tuple
                );
                tx.commit();
            };
            return _spsc_queue.try_enqueue(std::make_tuple(std::move(task), std::move(callback)));
        }
    private:
        void create_table_if_not_exists()
        {
            exec_sync(db::table::logs::create_table);
            exec_sync(db::table::orders::create_table);
            exec_sync(db::table::trades::create_table);
            exec_sync(db::table::positions::create_table);
            exec_sync(db::table::risk_indicators::create_table);
        }
        void prepare_stmt()
        {
            _async_engine.prepare(std::format("insert_{}", table::logs::table_name).c_str(), table::logs::insert.data());
            _async_engine.prepare(std::format("insert_{}", table::orders::table_name).c_str(), table::orders::insert.data());
            _async_engine.prepare(std::format("insert_{}", table::trades::table_name).c_str(), table::trades::insert.data());
            _async_engine.prepare(std::format("insert_{}", table::positions::table_name).c_str(), table::positions::insert.data());
            _async_engine.prepare(std::format("insert_{}", table::risk_indicators::table_name).c_str(), table::risk_indicators::insert.data());
            _sync_engine.prepare(std::format("insert_{}", table::logs::table_name).c_str(), table::logs::insert.data());
            _sync_engine.prepare(std::format("insert_{}", table::orders::table_name).c_str(), table::orders::insert.data());
            _sync_engine.prepare(std::format("insert_{}", table::trades::table_name).c_str(), table::trades::insert.data());
            _sync_engine.prepare(std::format("insert_{}", table::positions::table_name).c_str(), table::positions::insert.data());
            _sync_engine.prepare(std::format("insert_{}", table::risk_indicators::table_name).c_str(), table::risk_indicators::insert.data());
        }
        moodycamel::ReaderWriterQueue<std::tuple<std::function<void()>, std::function<void(bool)>>> _spsc_queue;
        pqxx::connection _async_engine;
        pqxx::connection _sync_engine;
        std::unique_ptr<std::jthread> _busy_worker;
    };
};