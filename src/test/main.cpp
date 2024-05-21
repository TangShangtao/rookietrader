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

class MDReceiver : public MDApi
{
public:
    // Event Callback
    MDReceiver(
        const std::string& eventUrl,
        const std::string& rpcUrl,
        const std::string& loggerName,
        const std::string& logMode
    )
        :   MDApi(eventUrl, rpcUrl, loggerName, logMode)
    {
        Init();
        SendPrepareMDReq();
    }
    ~MDReceiver() override
    {
        std::cout << "MDReceiver ~MDReceiver" << std::endl;
        Join();
    }
    void OnMDReady(const MDReady* event) override
    {
        std::cout << "MDReceiver OnMDReady: " << std::endl;
    }
    void OnTick(const Tick* event) override
    {
        std::cout << "MDReceiver OnTick: " << event->instrumentID.data() << "lastprice: " << event->lastPrice << std::endl;
    }
    // Rpc Callback
    void OnPrepareMDRsp(const PrepareMDRsp* rsp) override
    {
        std::cout << "MDReceiver OnPrepareMDRsp" << std::endl;        
        std::vector<std::string> subInstruments;
        subInstruments.push_back("IH2406");
        subInstruments.push_back("IH2405");
        SendSubTickReq(ExchangeID::SHFE, subInstruments);
    }
    void OnSubTickRsp(const SubTickRsp* rsp) override
    {
        std::cout << "MDReceiver OnSubTickRsp" << std::endl;
    }    
};

int main()
{
    std::ifstream ifs("config.json");
    if (ifs)
    {
        auto config = nlohmann::json::parse(ifs).at("MDApi");
        MDReceiver recever(
            config.at("eventUrl").get<const std::string>(),
            config.at("rpcUrl").get<const std::string>(),
            config.at("loggerName").get<const std::string>(),
            config.at("logMode").get<const std::string>()
        );
    }
    else
    {
        std::cout << "config.json not found in current working directory" << std::endl;
    }
}