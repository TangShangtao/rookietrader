// rpc api to tdservice
// use tdapi to execute trade commands
# pragma once
#include "protocol.h"
#include "tools/logger.h"
#include "nng/nng.h"
#include "nlohmann/json.hpp"
#include <string>
#include <vector>
#include <unordered_set>

namespace rookietrader
{
class TDApi
{
public:
    TDApi(
        const std::string& eventUrl,
        const std::string& rpcUrl,
        const std::string& loggerName,
        const std::string& logMode
    );
    virtual ~TDApi();
    // call Subscribe to subscribe push events
    void Subscribe(EventType event);
    // call Init to start receiving events, call it at end of main thread
    void Init();
    
};
};