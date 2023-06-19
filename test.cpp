
#include "TradingSystem/TradingPlatformConnector/TradeGateway/CTPGateway/CTPGateway.h"
#include "DataKit/CfgLoader.h"
#include "tools/Logger.h"
#include <iostream>
#define print std::cout << 
#define endl << std::endl



int main()
{
    Variant* cfg = CfgLoader::load_from_file("CTPConnect.json");
    CTPGateway gateway;
    gateway.init(cfg);
    gateway.connect();
    gateway.login();
    gateway.join();
    // Logger::info("hello world");
    
}   