#include "nlohmann/json.hpp"
#include "magic_enum/magic_enum.hpp"
#include "tools/logger.h"
#include "protocol.h"
#include "api/mdapi.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace rookietrader;

class MDReceiver : public MDSpi
{
public:
    // Event Callback
    void OnMDReady(const MDReady* event) override
    {
        
    }
    void OnTick(const Tick* event) override
    {

    }
    // Rpc Callback
    void OnPrepareMDRsp(const PrepareMDRsp* rsp) override
    {

    }
    void OnSubTickRsp(const SubTickRsp* rsp) override
    {
        
    }    
};

int main()
{
    std::ifstream f("config.json");
    nlohmann::json j = nlohmann::json::parse(f);
    auto loggerConfig = j.at("MDService").at("logger");
    Logger logger(loggerConfig.at("name").get<std::string>(), loggerConfig.at("logMode").get<std::string>());
    MDReady ready(1);
    logger.info("{}", ready.DebugInfo());
}