#include "adapter.h"
#include "impl/ctp_adapter.h"
#include "impl/emt_adapter.h"
#include "impl/gateway_adapter.h"
namespace rk::adapter
{
    std::unique_ptr<MDAdapter> create_md_adapter(
        MDAdapter::PushDataCallbacks push_data_callbacks,
        config_type::MDAdapterConfig config
    )
    {
        if (config.adapter_name == "CTP")
        {
            return std::make_unique<CTPMDAdapter>(std::move(push_data_callbacks), std::move(config));
        }
        if (config.adapter_name == "EMT")
        {
            return std::make_unique<EMTMDAdapter>(std::move(push_data_callbacks), std::move(config));
        }
        if (config.adapter_name == "Gateway")
        {
            return std::make_unique<MDGatewayAdapter>(std::move(push_data_callbacks), std::move(config));
        }
        else return nullptr;
    }
    std::unique_ptr<TDAdapter> create_td_adapter(
        TDAdapter::PushDataCallbacks push_data_callbacks,
        config_type::TDAdapterConfig config
    )
    {
        if (config.adapter_name == "CTP")
        {
            return std::make_unique<CTPTDAdapter>(std::move(push_data_callbacks), std::move(config));
        }
        if (config.adapter_name == "EMT")
        {
            return std::make_unique<EMTTDAdapter>(std::move(push_data_callbacks), std::move(config));
        }
        else return nullptr;
    }

}
