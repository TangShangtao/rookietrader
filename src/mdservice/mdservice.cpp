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
    logger(loggerName, logMode),
    running(false)
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
    logger.debug("MDService::MDService,called");
    SignalHandler::SignalLogCallback logCallback = [&](const std::string& msg)
    {
        logger.info(msg);
    };
    SignalHandler::ExitCallback exitCallback = [&](int signal)
    {
        logger.info(fmt::format("MDService::ExitCallback,signal {}; exit", signal));
        Stop();
        exit(signal);
    };
    SignalHandler::RegisterSignalCallbacks(logCallback, exitCallback);
}
MDService::~MDService()
{
    logger.debug("MDService::~MDService,called");
}
void MDService::Run()
{
    running = true;
    handleReqThread = std::make_shared<std::thread>(std::bind(&MDService::HandleReq, this));
    WaitReq();
}
void MDService::Stop()
{
    running = false;
    OnDisconnect();
    nngRes = nng_close(eventSock);
    if (nngRes != 0)
    {
        logger.debug(fmt::format("MDService::Stop;nng_close eventSock {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
    }
    logger.debug("MDService::nng_close(eventSock);called");
    nngRes = nng_close(rpcSock);
    if (nngRes != 0)
    {
        logger.debug(fmt::format("MDService::Stop;nng_close rpcSock {}; res {}; msg {}", rpcUrl, nngRes, nng_strerror(nngRes)));
    }
    logger.debug("MDService::nng_close(rpcSock);called");
    if (handleReqThread->joinable())
    {
        handleReqThread->join();
    }
    logger.debug("MDService::Stop;called");
}
void MDService::HandleReq()
{
    logger.debug("MDService::HandleReq,waiting PrepareMDReq");
    RPCReqHeader* buf = nullptr;
    size_t sz;
    while (running)
    {
        nngRes = nng_recv(rpcSock, reinterpret_cast<void*>(&buf), &sz, NNG_FLAG_ALLOC);
        if (nngRes != 0)
        {
            logger.error(fmt::format("MDService::HandleReq,nng_recv res {}; msg {}", nngRes, nng_strerror(nngRes)));
            break;
        }
        switch (buf->rpc)
        {
            case RPCType::PrepareMD:
            {
                if (prepareMDRpcID != 0)
                {
                    logger.error("MDService::HandleReq,prepareMDReq received more than once; {}", buf->DebugInfo());
                }
                else
                {
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
                break;
            }
            case RPCType::SubTick:
            {
                if (prepareMDRpcID == 0)
                {
                    logger.error("MDService::HandleReq,nng_recv received req; but not PrepareMDReq; {}", buf->DebugInfo());
                    continue;
                }
                else
                {
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
                break;
            }
            default:
            {
                logger.error("MDService::HandleReq,unsupported req received; {}", buf->DebugInfo());
                break;
            }
        }

    }
    nng_free(buf, sz);
    logger.debug("MDService::HandleReq,HandleReq finish");
} 

void MDService::WaitReq()
{
    if (handleReqThread->joinable())
    {
        logger.debug("MDService::WaitReq,joining in handleReqThread");
        handleReqThread->join();
        logger.debug("MDService::WaitReq,finish joining handleReqThread");
    }

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
