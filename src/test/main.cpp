#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <string>
int main()
{
    std::ifstream f("config.json");
    nlohmann::json j = nlohmann::json::parse(f);
    std::cout << j.at("marketdata").get<std::string>() << std::endl;
}