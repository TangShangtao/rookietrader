#include "api/mdapi.h"
#include "nng/protocol/reqrep0/req.h"
#include "nng/protocol/pubsub0/sub.h"
#include <functional>
namespace rookietrader
{
MDApi::MDApi(const nlohmann::json& config)
    :   eventUrl(config.at("eventUrl").get<std::string>()),
        rpcUrl(config.at("rpcUrl").get<std::string>()),
        logger(config.at("logger").at("name").get<std::string>(), 
        config.at("logger").at("logMode").get<std::string>())
{
    eventSock = NNG_SOCKET_INITIALIZER;
    rpcSock = NNG_SOCKET_INITIALIZER;
    nngRes = nng_sub0_open(&eventSock);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDApi::MDApi,nng_sub0_open res {}; msg {}", nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_socket_set(eventSock, NNG_OPT_SUB_SUBSCRIBE, "", 0);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDApi::MDApi,nng_socket_set res {}; msg {}", nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_req0_open(&rpcSock);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDApi::MDApi,nng_req0_open res {}; msg {}", nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_dial(eventSock, eventUrl.c_str(), nullptr, 0);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDApi::MDApi,nng_dial {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_dial(rpcSock, rpcUrl.c_str(), nullptr, 0);
    if (nngRes < 0)
    {
        logger.debug(fmt::format("MDApi::MDApi,nng_dial {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    handleRspThread = std::make_shared<std::thread>(std::bind(&MDApi::HandleRsp, this));
    handleEventThread = std::make_shared<std::thread>(std::bind(&MDApi::HandleEvent, this));
    logger.debug("MDApi::MDApi,called");
}

MDApi::~MDApi()
{
    handleRspThread->join();
    handleEventThread->join();
    logger.debug("MDApi::~MDApi,called");

}

void MDApi::RegisterSpi(MDSpi* spi)
{
    if (spi == nullptr)
    {
        logger.error("MDApi::RegisterSpi,spi is nullptr");
        exit(-1);
    }
    this->spi = spi;
    logger.debug("MDApi::RegisterSpi,called");
}

int MDApi::SendPrepareMDReq()
{
    PrepareMDReq req(++rpcID);
    nngRes = nng_send(rpcSock, reinterpret_cast<void*>(&req), sizeof(PrepareMDReq), 0);
    if (nngRes < 0)
    {
        logger.error(fmt::format("MDApi::SendPrepareMDReq,nng_send {}; res {}; msg {}", rpcUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }    
    logger.debug("MDApi::SendPrepareMDReq,{}", req.DebugInfo());
    prepareMDRpcID = rpcID;
    return rpcID;
}

int MDApi::SendSubTickReq(ExchangeID exchange, std::vector<std::string>& instruments)
{
    SubTickReq req(++rpcID, exchange);
    void* buf = req.CreateReqBuf(instruments);
    nngRes = nng_send(rpcSock, buf, sizeof(req.byteSize), 0);
    if (nngRes < 0)
    {
        logger.error(fmt::format("MDApi::SendSubTickReq,nng_send {}; res {}; msg {}", rpcUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }  
    req.ReleaseReqBuf(buf);
    logger.debug("MDApi::SendSubTickReq,{}", req.DebugInfo());
    subTickRpcID = rpcID;
    return rpcID;
}


void MDApi::HandleRsp()
{
    logger.info("MDApi::HandleRsp,waiting PrepareMDRsp");
    RPCRspHeader* buf = nullptr;
    size_t sz;
    while (prepareMDRpcID == 0)
    {
        nngRes = nng_recv(rpcSock, reinterpret_cast<void*>(&buf), &sz, NNG_FLAG_ALLOC);
        if (buf->rpc != RPCType::PrepareMD)
        {
            logger.error("MDApi::HandleRsp,nng_recv received req; but not PrepareMDRsp; {}", buf->DebugInfo());
            continue;
        }
        PrepareMDRsp* rsp = reinterpret_cast<PrepareMDRsp*>(buf);
        prepareMDRpcID = rsp->rpcID;
        logger.info("MDApi::HandleRsp,PrepareMDRsp received; {}", rsp->DebugInfo());
    
        spi->OnPrepareMDRsp(rsp);

        logger.info("MDApi::HandleRsp,OnPrepareMDRsp called;");
    }
    logger.info("MDApi::HandleRsp,waiting SubTickRsp;");
    while (subTickRpcID == 0)
    {
        nngRes = nng_recv(rpcSock, reinterpret_cast<void*>(&buf), &sz, NNG_FLAG_ALLOC);
        if (buf->rpc != RPCType::SubTick)
        {
            logger.error("MDApi::HandleRsp,nng_recv received req {}; but not SubTickReq", buf->DebugInfo());
            continue;
        }
        SubTickRsp* rsp = reinterpret_cast<SubTickRsp*>(buf);
        subTickRpcID = rsp->rpcID;
        logger.info("MDApi::HandleRsp,SubTickRsp received; {}", rsp->DebugInfo());
        
        spi->OnSubTickRsp(rsp);
    
        logger.info("MDApi::HandleRsp,OnSubTickRsp called;");
    }
    nng_free(buf, sz);
    logger.info("MDApi::HandleRsp,HandleRsp exit");    

}

void MDApi::HandleEvent()
{

}
};
