#include "DataKit/CommonMgr.h"
#include "DataKit/CfgLoader.h"
#include "TradingSystem/TradeGateway/CTPGateway/CTPGateway.h"
#include "Defines/TradeDefs/Entrust.hpp"
#include "Defines/TradeDefs/OrderInfo.hpp"
#include <iostream>
#define print std::cout<<
#define endl <<std::endl
int main()
{
    Variant* cfg = CfgLoader::load_from_file("CTPConnect.json");
    CTPGateway* gateway = new CTPGateway();
    CommonMgr* cmgr = new CommonMgr();
    cmgr->load_sessions("dist/common/sessions.json");
    cmgr->load_commodities("dist/common/commodities.json");
    cmgr->load_contracts("dist/common/contracts.json");
    cmgr->load_holidays("dist/common/holidays.json");
    gateway->add_cmgr(cmgr);

    gateway->init(cfg);
    gateway->connect();
    gateway->login();
    gateway->confirm();

    Entrust* entrust = Entrust::create("rb2310", 1, 100000, "SHFE");
    entrust->set_price_type(PT_AnyPrice);
    entrust->set_offset_type(OT_Open);
    entrust->set_direction(DT_Long);
    gateway->order_insert(entrust);

    gateway->join();



}