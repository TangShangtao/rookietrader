#include "mdservice.h"
#include <fstream>
namespace rookietrader
{
// set communicate url, read basic config from config.json
MDService::MDService(
    const std::string& eventUrl, 
    const std::string& rpcUrl, 
    const std::string& accountID, 
    const std::string& password,
    const std::string& frontAddr,
    const std::string& loggerName,
    const std::string& logMode
)   :   
    eventUrl(eventUrl),
    rpcUrl(rpcUrl),
    accountID(accountID),
    password(password),
    frontAddr(frontAddr),
    logger(loggerName, logMode)
{
    eventSock = NNG_SOCKET_INITIALIZER;
    rpcSock = NNG_SOCKET_INITIALIZER;
    nngRes = nng_pub0_open(&eventSock);
    if (nngRes != 0)
    {
        logger.debug(fmt::format("MDService::MDService,nng_pub0_open res {}; msg {}", nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_rep0_open(&rpcSock);
    if (nngRes != 0)
    {
        logger.debug(fmt::format("MDService::MDService,nng_rep0_open res {}; msg {}", nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_listen(eventSock, eventUrl.c_str(), nullptr, 0);
    if (nngRes != 0)
    {
        logger.debug(fmt::format("MDService::MDService,nng_listen {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_listen(rpcSock, rpcUrl.c_str(), nullptr, 0);
    if (nngRes != 0)
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
    logger.debug("MDService::HandleReq,waiting PrepareMDReq");
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
        logger.debug("MDService::HandleReq,PrepareMDReq received; {}", req->DebugInfo());
        bool res = OnPrepareMDReq(req);
        logger.debug("MDService::HandleReq,OnPrepareMDReq called;");
        if (res == false)
        {
            logger.error("MDService::HandleReq,OnPrepareMDReq return false;");
            exit(-1);
        }
    }

    logger.debug("MDService::HandleReq,waiting SubTickReq;");
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
        logger.debug("MDService::HandleReq,SubTickReq received; {}", req->DebugInfo());
        bool res = OnSubTickReq(req);
        logger.debug("MDService::HandleReq,OnSubTickReq called;");
        if (res == false)
        {
            logger.error("MDService::HandleReq,OnSubTickReq return false;");
            exit(-1);
        }
    }
    nng_free(buf, sz);
    logger.debug("MDService::HandleReq,HandleReq finish");
} 

void MDService::WaitReq()
{
    handleReqThread->join();
}

void MDService::SendPrepareMDRsp(bool isSucc, const std::string& msg)
{
    PrepareMDRsp rsp(prepareMDRpcID, isSucc, msg);
    logger.debug("MDService::SendPrepareMDRsp,{}", rsp.DebugInfo());
    nngRes = nng_send(rpcSock, reinterpret_cast<void*>(&rsp), sizeof(PrepareMDRsp), 0);
    if (nngRes != 0)
    {
        logger.error(fmt::format("MDService::SendPrepareMDRsp,nng_send {}; res {}; msg {}", rpcUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    logger.debug("MDService::SendPrepareMDRsp,called");
}

void MDService::SendSubTickRsp(bool isSucc, const std::string& msg)
{
    SubTickRsp rsp(subTickRpcID, isSucc, msg);
    logger.debug("MDService::SendSubTickRsp,{}", rsp.DebugInfo());
    nngRes = nng_send(rpcSock, reinterpret_cast<void*>(&rsp), sizeof(PrepareMDRsp), 0);
    if (nngRes != 0)
    {
        logger.error(fmt::format("MDService::SendSubTickRsp,nng_send {}; res {}; msg {}", rpcUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    logger.debug("MDService::SendSubTickRsp,called");
}

// generate MDReady event, publish to subscribers
void MDService::PublishMDReady()
{
    MDReady mdReady(prepareMDRpcID);
    logger.debug("MDService::PublishMDReady,{}", mdReady.DebugInfo());
    nngRes = nng_send(eventSock, reinterpret_cast<void*>(&mdReady), sizeof(MDReady), 0);
    if (nngRes != 0)
    {
        logger.error(fmt::format("MDService::PublishMDReady,nng_send {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    logger.debug("MDService::PublishMDReady,called");
}
// generate Tick event, publish to subscribers
void MDService::PublishTick(const Tick& tick)
{
    logger.debug("MDService::PublishTick,{}", tick.DebugInfo());
    nngRes = nng_send(eventSock, const_cast<Tick*>(&tick), sizeof(Tick), 0);
    if (nngRes != 0)
    {
        logger.debug(fmt::format("MDService::PublishTick,nng_send {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    logger.debug("MDService::PublishTick,called");
}

};
