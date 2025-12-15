//
// Created by root on 2025/10/24.
//

#include "algo.h"
#include "impl/twap.h"


namespace rk::algo
{
    std::shared_ptr<Algo> create_algo(
        EngineImpl& engine,
        const data_type::Symbol& symbol,
        const std::string& algo_name,
        const std::string& algo_param_json
    )
    {
        if (algo_name == "TWAP")
        {
            return std::make_shared<Twap>(
                engine,
                symbol,
                algo_param_json
            );
        }
        return nullptr;
    }
};