#include "nlohmann/json.hpp"
#include "magic_enum/magic_enum.hpp"
#include "tools/logger.h"
#include "protocol.h"
#include "api/mdapi.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace rookietrader;

class MDReceiver : public MDSpi
{
public:
    // Event Callback
    MDReceiver(const nlohmann::json& config)
        :   api(config)
    {
        api.RegisterSpi(this);
        api.Init();
        api.SendPrepareMDReq();
    }
    void OnMDReady(const MDReady* event) override
    {
        std::cout << event->DebugInfo() << std::endl;
        std::vector<std::string> subInstruments;
        subInstruments.push_back("IH2406");
        api.SendSubTickReq(ExchangeID::SHFE, subInstruments);
    }
    void OnTick(const Tick* event) override
    {

    }
    // Rpc Callback
    void OnPrepareMDRsp(const PrepareMDRsp* rsp) override
    {
        std::cout << rsp->DebugInfo() << std::endl;
    }
    void OnSubTickRsp(const SubTickRsp* rsp) override
    {
        std::cout << rsp->DebugInfo() << std::endl;
    }    
private:
    MDApi api;
};

int main()
{
    std::ifstream f("config.json");
    nlohmann::json j = nlohmann::json::parse(f);
    // auto loggerConfig = j.at("MDService").at("logger");
    // Logger logger(loggerConfig.at("name").get<std::string>(), loggerConfig.at("logMode").get<std::string>());
    // MDReady ready(1);
    // logger.info("{}", ready.DebugInfo());
    MDReceiver recver(j.at("MDApi"));

}