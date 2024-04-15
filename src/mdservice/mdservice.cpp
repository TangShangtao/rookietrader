#include "mdservice.h"

namespace rookietrader
{
// set communicate url, read basic config from config.json
MDService::MDService(const nlohmann::json& config)
    :   eventUrl(config.at("eventUrl").get<std::string>()), 
        rpcUrl(config.at("rpcUrl").get<std::string>()), 
        mdName(config.at("mdName").get<std::string>()),
        accountID(config.at("accountID").get<std::string>()),
        password(config.at("password").get<std::string>()),
        frontAddr(config.at("frontAddr").get<std::string>()),
        logger(config.at("logger").at("name").get<std::string>(), 
        config.at("logger").at("logMode").get<std::string>())
{
    eventSock = NNG_SOCKET_INITIALIZER;
    rpcSock = NNG_SOCKET_INITIALIZER;
    nngRes = nng_pub0_open(&eventSock);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDService::MDService,nng_pub0_open res {}; msg {}", nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_rep0_open(&rpcSock);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDService::MDService,nng_rep0_open res {}; msg {}", nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_listen(eventSock, eventUrl.c_str(), nullptr, 0);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDService::MDService,nng_listen {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_listen(rpcSock, rpcUrl.c_str(), nullptr, 0);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDService::MDService,nng_listen {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    handleReqThread = std::make_shared<std::thread>(std::bind(&MDService::HandleReq, this));
}
MDService::~MDService()
{
    handleReqThread->join();
    logger.debug("MDService::~MDService,called");
}
void MDService::HandleReq()
{
    
}
// generate MDReady event, publish to subscribers
void MDService::OnMDReady()
{
    MDReady mdReady(prepareRpcID);
    std::strncpy(mdReady.url, eventUrl.c_str(), sizeof(mdReady.url)-1);
    logger.debug("MDService::OnMDReady,{}", mdReady.DebugInfo());
    nngRes = nng_send(eventSock, &mdReady, sizeof(MDReady), 0);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDService::OnMDReady,nng_send {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
}
// generate Tick event, publish to subscribers
void MDService::OnTick(const Tick& tick)
{
    logger.debug("MDService::OnTick,{}", tick.DebugInfo());
    nngRes = nng_send(eventSock, const_cast<Tick*>(&tick), sizeof(Tick), 0);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDService::OnTick,nng_send {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
}

};
