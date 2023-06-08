#include "HisDataReplayer/CommonMgr.h"


#include <iostream>
#include <string>
#define print std::cout << 
#define endl << std::endl


int main()
{
    CommonMgr cmgr;
    cmgr.load_sessions("dist/common/sessions.json");
    cmgr.load_commodities("dist/common/commodities.json");

    cmgr.load_contracts("dist/common/contracts.json");
    cmgr.load_holidays("dist/common/holidays.json");
    

}