#include "Defines/CommonDefs/CommodityInfo.hpp"

#include <iostream>
#include <string>

#define print std::cout << 
#define endl << std::endl


USING_NS;


int main()
{
    CommodityInfo* c = CommodityInfo::create("IF", "CFFEX", "IF", "09:15-11:30,13:00-15:15", "IF");

    int a = 1;
}