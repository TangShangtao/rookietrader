#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"
#include "implements/mdctp.h"

using namespace rookietrader;
int main() 
{
    std::ifstream ifs("config.json");
    if (ifs)
    {
        auto config = nlohmann::json::parse(ifs).at("MDService");
        auto mdName = config.at("mdName").get<const std::string>();
        if (mdName == "MDCTP")
        {
            MDCTP mdctp(
                config.at("eventUrl").get<const std::string>(),
                config.at("rpcUrl").get<const std::string>(),
                config.at("accountID").get<const std::string>(),
                config.at("password").get<const std::string>(),
                config.at("frontAddr").get<const std::string>(),
                config.at("loggerName").get<const std::string>(),
                config.at("logMode").get<const std::string>()
            );
        }
        else 
        {
            std::cout << "unsupported mdName in config.json" << std::endl;
        }
    }
    else
    {
        std::cout << "config.json not found in current working directory" << std::endl;
    }
    
    
    

    
}