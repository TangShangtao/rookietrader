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
        std::cout << "OnMDReady: " << event->DebugInfo() << std::endl;
    }
    void OnTick(const Tick* event) override
    {
        std::cout << "OnTick: " << event->instrumentID << "lastprice: " << event->lastPrice << std::endl;
    }
    // Rpc Callback
    void OnPrepareMDRsp(const PrepareMDRsp* rsp) override
    {
        std::cout << "OnPrepareMDRsp: " << rsp->DebugInfo() << std::endl;
        std::vector<std::string> subInstruments;
        subInstruments.push_back("IH2406");
        subInstruments.push_back("IH2405");
        api.SendSubTickReq(ExchangeID::SHFE, subInstruments);
    }
    void OnSubTickRsp(const SubTickRsp* rsp) override
    {
        std::cout << "OnSubTickRsp" << rsp->DebugInfo() << std::endl;
    }    
private:
    MDApi api;
};

int main()
{
    std::ifstream f("config.json");
    nlohmann::json j = nlohmann::json::parse(f);
    MDReceiver recever(j.at("MDApi"));

}