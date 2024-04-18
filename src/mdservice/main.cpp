#include <fstream>
#include "nlohmann/json.hpp"
#include "implements/mdctp.h"

using namespace rookietrader;
int main() 
{
    std::ifstream f("config.json");
    nlohmann::json j = nlohmann::json::parse(f);
    MDCTP mdctp(j.at("MDService"));
}