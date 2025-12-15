#include "impl/ctp_gateway.h"
#include "impl/emt_gateway.h"
namespace rk::gateway
{
    std::unique_ptr<Gateway> create_gateway(
        std::shared_ptr<event::EventLoop> event_loop,
        config_type::GatewayConfig config
    )
    {
        if (config.gateway_name == "CTP")
        {
            return std::make_unique<CTPGateway>(std::move(event_loop), std::move(config));
        }
        if (config.gateway_name == "EMT")
        {
            return std::make_unique<EMTGateway>(std::move(event_loop), std::move(config));
        }
        else return nullptr;

    }
};