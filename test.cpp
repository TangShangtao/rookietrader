
// #include "TradingSystem/TradingPlatformConnector/TradeGateway/CTPGateway/CTPGateway.h"
// #include "DataKit/CfgLoader.h"
// #include "tools/Logger.h"
#include "Defines/TradeDefs/TradeClass.hpp"
#include "Defines/ConstantDefs/Marcos.h"
#include <iostream>
#define print std::cout << 
#define endl << std::endl
USING_NS;


int main()
{
    // Variant* cfg = CfgLoader::load_from_file("CTPConnect.json");
    // CTPGateway gateway;
    // gateway.init(cfg);
    // gateway.connect();
    // gateway.login();
    // gateway.join();
    // Logger::info("hello world");
    Entrust* pEntrust = Entrust::create();
    print pEntrust->m_bIsNet endl;
    pEntrust->release();
}   