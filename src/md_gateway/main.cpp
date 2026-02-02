#include "md_gateway.h"
#include "config_type.h"
using namespace rk;
int main()
{
    auto md_gateway = gateway::MDGateway(config_type::MDGatewayConfig::load_config_file("emt_gateway.toml"));
    md_gateway.start_trading();

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}