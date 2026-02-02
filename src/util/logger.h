#pragma once
#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/FileSink.h"
#include "quill/sinks/ConsoleSink.h"
#include <filesystem>


namespace rk::util
{
    inline quill::Logger* g_logger = nullptr;
    inline void init_logger(const std::string& log_file_path, const std::string& logger_name)
    {
        quill::Backend::start();

        // Frontend
        auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(
            log_file_path,
            []()
            {
                quill::FileSinkConfig cfg;
                cfg.set_open_mode('w');
                cfg.set_filename_append_option(quill::FilenameAppendOption::StartDateTime);
                return cfg;
            }(),
            quill::FileEventNotifier{});

        g_logger = quill::Frontend::create_or_get_logger("root", std::move(file_sink));
    }
    inline quill::Logger* get_logger() {
        if (!g_logger)
        {
            quill::Backend::start();
            auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_id_1");

            g_logger = quill::Frontend::create_or_get_logger("root", std::move(console_sink));
            g_logger->set_log_level(quill::LogLevel::TraceL3);
        }
        return g_logger;
    }

}
#define RK_LOG_DEBUG(...) LOG_DEBUG(rk::util::get_logger(), __VA_ARGS__)
#define RK_LOG_INFO(...)  LOG_INFO(rk::util::get_logger(), __VA_ARGS__)
#define RK_LOG_WARN(...)  LOG_WARNING(rk::util::get_logger(), __VA_ARGS__)
#define RK_LOG_ERROR(...) LOG_ERROR(rk::util::get_logger(), __VA_ARGS__)
