#include "nlohmann/json.hpp"
#include "tools/logger.h"
#include <fstream>
#include <iostream>
#include <string>
int main()
{
    std::ifstream f("config.json");
    nlohmann::json j = nlohmann::json::parse(f);
    auto loggerConfig = j.at("MDService").at("logger");
    rookietrader::Logger logger(loggerConfig.at("name").get<std::string>(), loggerConfig.at("logMode").get<std::string>());
    logger.debug("aaa");
}