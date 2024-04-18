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
    logger.debug("MDService::MDService,called");
}
MDService::~MDService()
{
    logger.debug("MDService::~MDService,called");
}
void MDService::HandleReq()
{
    logger.info("MDService::HandleReq,waiting PrepareMDReq");
    RPCReqHeader* buf = nullptr;
    size_t sz;
    while (prepareMDRpcID == 0)
    {
        nngRes = nng_recv(rpcSock, reinterpret_cast<void*>(&buf), &sz, NNG_FLAG_ALLOC);
        if (buf->rpc != RPCType::PrepareMD)
        {
            logger.error("MDService::HandleReq,nng_recv received req; but not PrepareMDReq; {}", buf->DebugInfo());
            continue;
        }
        PrepareMDReq* req = reinterpret_cast<PrepareMDReq*>(buf);
        prepareMDRpcID = req->rpcID;
        logger.info("MDService::HandleReq,PrepareMDReq received; {}", req->DebugInfo());
        bool res = OnPrepareMDReq(req);
        if (res == false)
        {
            logger.error("MDService::HandleReq,OnPrepareMDReq return false;");
            exit(-1);
        }
        logger.info("MDService::HandleReq,OnPrepareMDReq called;");
    }
    logger.info("MDService::HandleReq,waiting SubTickReq;");
    while (subTickRpcID == 0)
    {
        nngRes = nng_recv(rpcSock, reinterpret_cast<void*>(&buf), &sz, NNG_FLAG_ALLOC);
        if (buf->rpc != RPCType::SubTick)
        {
            logger.error("MDService::HandleReq,nng_recv received req {}; but not SubTickReq", buf->DebugInfo());
            continue;
        }
        SubTickReq* req = reinterpret_cast<SubTickReq*>(buf);
        subTickRpcID = req->rpcID;
        logger.info("MDService::HandleReq,SubTickReq received; {}", req->DebugInfo());
        bool res = OnSubTickReq(req);
        if (res == false)
        {
            logger.error("MDService::HandleReq,Prepare return false;");
            SendPrepareMDRsp(false, "MDService OnSubTickReq return false");
            exit(-1);
        }
        SendPrepareMDRsp(true, "");
        logger.info("MDService::HandleReq,OnSubTickReq called;");
    }
    nng_free(buf, sz);
    logger.info("MDService::HandleReq,HandleReq exit");
} 

void MDService::WaitReq()
{
    handleReqThread->join();
}

void MDService::SendPrepareMDRsp(bool isError, const std::string& msg)
{
    PrepareMDRsp rsp(prepareMDRpcID, isError, msg);
    logger.debug("MDService::SendPrepareMDRsp,{}", rsp.DebugInfo());
    nngRes = nng_send(rpcSock, reinterpret_cast<void*>(&rsp), sizeof(PrepareMDRsp), 0);
    if (nngRes < 0)
    {
        logger.error(fmt::format("MDService::SendPrepareMDRsp,nng_send {}; res {}; msg {}", rpcUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    logger.debug("MDService::SendPrepareMDRsp,called");

}

// generate MDReady event, publish to subscribers
void MDService::PublishMDReady()
{
    MDReady mdReady(prepareMDRpcID);
    logger.debug("MDService::OnMDReady,{}", mdReady.DebugInfo());
    nngRes = nng_send(eventSock, reinterpret_cast<void*>(&mdReady), sizeof(MDReady), 0);
    if (nngRes < 0)
    {
        logger.error(fmt::format("MDService::OnMDReady,nng_send {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    logger.debug("MDService::PublishMDReady,called");
}
// generate Tick event, publish to subscribers
void MDService::PublicTick(const Tick& tick)
{
    logger.debug("MDService::PublicTick,{}", tick.DebugInfo());
    nngRes = nng_send(eventSock, const_cast<Tick*>(&tick), sizeof(Tick), 0);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDService::PublicTick,nng_send {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    logger.debug("MDService::PublicTick,called");
}

};
