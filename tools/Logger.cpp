#include "Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>


bool Logger::m_bInited = false;
Logger::LoggerPtr Logger::m_pLogger = nullptr;
char Logger::m_msg[MAX_MSG_LEN] = { 0 };


void Logger::init()
{
    if (m_bInited)
    {
        return;
    }
    std::vector<spdlog::sink_ptr> sinks;
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sinks.emplace_back(console_sink);
    // auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt", true);
    // sinks.emplace_back(file_sink);
    auto logger = std::make_shared<spdlog::logger>("root", begin(sinks), end(sinks));
    logger->set_level(spdlog::level::debug);
    spdlog::register_logger(logger);

    m_pLogger = spdlog::get("root");
    if (m_pLogger != nullptr)
    {
        m_bInited = true;
    }
}
Logger::LoggerPtr Logger::get_logger(const char* logger_name)
{
    if (!m_bInited)
    {
        return nullptr;
    }
    return spdlog::get(logger_name);
}