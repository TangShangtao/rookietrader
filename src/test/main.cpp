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
    MDReceiver(const std::string& configPath)
        :   api(configPath)
    {
        api.RegisterSpi(this);
        api.Init();
        api.SendPrepareMDReq();
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
        std::cout << "MDReceiver OnPrepareMDRsp" << std::endl;        std::vector<std::string> subInstruments;
        subInstruments.push_back("IH2406");
        subInstruments.push_back("IH2405");
        api.SendSubTickReq(ExchangeID::SHFE, subInstruments);
    }
    void OnSubTickRsp(const SubTickRsp* rsp) override
    {
        std::cout << "MDReceiver OnSubTickRsp" << std::endl;
    }    
private:
    MDApi api;
};

int main()
{
    MDReceiver recever("config.json");

}