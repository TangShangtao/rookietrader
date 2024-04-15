#include "mdservice.h"

namespace rookietrader
{
// set communicate url, read basic config from config.json
MDService::MDService(nlohmann::json config)
    :   mdUrl(config.at("mdUrl").get<std::string>()), 
        mdName(config.at("mdName").get<std::string>()),
        accountID(config.at("accountID").get<std::string>()),
        password(config.at("password").get<std::string>()),
        frontAddr(config.at("frontAddr").get<std::string>()),
        logger(config.at("logger").at("name").get<std::string>(), 
        config.at("logger").at("logMode").get<std::string>())
{
    sock = NNG_SOCKET_INITIALIZER;
    nngRes = nng_pub0_open(&sock);
    logger.debug(nng_strerror(nngRes));
    nngRes = nng_listen(sock, mdUrl.c_str(), nullptr, 0);
    logger.debug(nng_strerror(nngRes));
}
// generate MDReady event, publish to subscribers
void MDService::OnMDReady()
{
    MDReady mdReady;
    // set
    logger.debug("OnMDReady,{}", mdReady.DebugInfo());
    nngRes = nng_send(sock, &mdReady, sizeof(MDReady), 0);
    logger.debug(nng_strerror(nngRes));
}
// generate Tick event, publish to subscribers
void MDService::OnTick(const Tick& tick)
{
    logger.debug("OnTick,{}", tick.DebugInfo());
    nngRes = nng_send(sock, const_cast<Tick*>(&tick), sizeof(Tick), 0);
}

};
