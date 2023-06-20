#include "DataKit/CfgLoader.h"
// #include "DataKit/CommonMgr.h"
// #include "tools/Logger.h"
#include "TradingSystem/TradeGateway/CTPGateway/CTPGateway.h"

#include <iostream>
#define print std::cout<<
#define endl <<std::endl
int main()
{
    Variant* cfg = CfgLoader::load_from_file("CTPConnect.json");
    CTPGateway* gateway = new CTPGateway();
    gateway->init(cfg);
    gateway->connect();
    gateway->join();

}