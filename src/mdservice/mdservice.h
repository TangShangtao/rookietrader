// connect to different fronts and publish marketdata
#pragma once
#include "protocol.h"
#include "tools/logger.h"
#include "nng/nng.h"
#include "nng/protocol/pubsub0/pub.h"
#include "nlohmann/json.hpp"

namespace rookietrader
{

class MDService
{
public:
    // set communicate url, read basic config from config.json
    MDService(nlohmann::json config);
    virtual ~MDService() = default;
    // connect and login to broker's marketdata front
    virtual bool Prepare() noexcept = 0;
    // generate MDReady event, publish to subscribers
    void OnMDReady();
   // subscribe market data of many instruments calling api only once
    virtual bool SubscribeMarketData(SubscribeMarketDataReq instrumentIDs) noexcept = 0;
    // generate Tick event, publish to subscribers
    void OnTick(const Tick& tick);
protected:
    // communicate url
    const std::string mdUrl;
    // unique name of this marketdata gateway
    const std::string mdName;
    // accountID given by broker
    const std::string accountID;
    // password given by broker
    const std::string password;
    // ip addr of broker's marketdata front
    const std::string frontAddr;
    // log api
    const Logger logger;
    // reqID used by md api
    int reqID = 0;
private:
    nng_socket sock;
    int nngRes;
    
};

};
