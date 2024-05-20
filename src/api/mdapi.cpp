#include "api/mdapi.h"
#include "nng/protocol/reqrep0/req.h"
#include "nng/protocol/pubsub0/sub.h"
#include <functional>
#include <fstream>
namespace rookietrader
{
MDApi::MDApi(const std::string& configPath)
{
    std::ifstream f(configPath);
    nlohmann::json config = nlohmann::json::parse(f);
    eventUrl = config.at("MDApi").at("eventUrl").get<std::string>();
    rpcUrl = config.at("MDApi").at("rpcUrl").get<std::string>();
    logger = Logger(config.at("MDApi").at("logger").at("name").get<std::string>(), config.at("MDApi").at("logger").at("logMode").get<std::string>());        
    eventSock = NNG_SOCKET_INITIALIZER;
    rpcSock = NNG_SOCKET_INITIALIZER;
    int nngRes;
    nngRes = nng_sub0_open(&eventSock);
    if (nngRes != 0)
    {
        logger.debug(fmt::format("MDApi::MDApi,nng_sub0_open res {}; msg {}", nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_socket_set(eventSock, NNG_OPT_SUB_SUBSCRIBE, "", 0);
    if (nngRes != 0)
    {
        logger.debug(fmt::format("MDApi::MDApi,nng_socket_set res {}; msg {}", nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_req0_open(&rpcSock);
    if (nngRes != 0)
    {
        logger.debug(fmt::format("MDApi::MDApi,nng_req0_open res {}; msg {}", nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_dial(eventSock, eventUrl.c_str(), nullptr, 0);
    if (nngRes != 0)
    {
        logger.debug(fmt::format("MDApi::MDApi,nng_dial {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    nngRes = nng_dial(rpcSock, rpcUrl.c_str(), nullptr, 0);
    if (nngRes != 0)
    {
        logger.debug(fmt::format("MDApi::MDApi,nng_dial {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }
    logger.debug("MDApi::MDApi,called");
}

MDApi::~MDApi()
{
    logger.debug("MDApi::~MDApi,called");

}

void MDApi::Init()
{
    handleEventThread = std::make_shared<std::thread>(std::bind(&MDApi::HandleEvent, this));
}

void MDApi::Join()
{
    logger.debug("MDApi::Join,Joining handleEventThread");
    if (handleEventThread != nullptr)
    {
        handleEventThread->join();
    }    
}

int MDApi::SendPrepareMDReq()
{
    PrepareMDReq req(++rpcID);
    int nngRes;
    nngRes = nng_send(rpcSock, reinterpret_cast<void*>(&req), sizeof(PrepareMDReq), 0);
    if (nngRes != 0)
    {
        logger.error(fmt::format("MDApi::SendPrepareMDReq,nng_send {}; res {}; msg {}", rpcUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }    
    logger.debug("MDApi::SendPrepareMDReq,{}", req.DebugInfo());
    
    RPCRspHeader* buf = nullptr;
    size_t sz;
    nngRes = nng_recv(rpcSock, reinterpret_cast<void*>(&buf), &sz, NNG_FLAG_ALLOC);
    if (nngRes != 0)
    {
        logger.error(fmt::format("MDApi::SendPrepareMDReq,nng_recv {}; res {}; msg {}", rpcUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }  
    if (buf->rpc != RPCType::PrepareMD)
    {
        logger.error("MDApi::SendPrepareMDReq,nng_recv received req; but not PrepareMDRsp; {}", buf->DebugInfo());
        exit(-1);
    }
    PrepareMDRsp* rsp = reinterpret_cast<PrepareMDRsp*>(buf);
    logger.info("MDApi::SendPrepareMDReq,PrepareMDRsp {}", rsp->DebugInfo());
    nng_free(buf, sz);

    OnPrepareMDRsp(rsp);
    
    logger.info("MDApi::SendPrepareMDReq,OnPrepareMDRsp called;");

       
    return rpcID;
}

int MDApi::SendSubTickReq(ExchangeID exchange, std::vector<std::string>& instruments)
{
    SubTickReq req(++rpcID, exchange, instruments.size());
    char* reqBuf = (char*)nng_alloc(req.byteSize);
    char* mover = reqBuf;
    std::memcpy(mover, &req, sizeof(req));
    mover += sizeof(req);
    for (int i = 0; i < req.counts; ++i)
    {
        std::memcpy(mover, instruments[i].c_str(), SubTickReq::instrumentIDLen-1);
        mover += SubTickReq::instrumentIDLen;
    }    
    int nngRes;
    nngRes = nng_send(rpcSock, reqBuf, req.byteSize, NNG_FLAG_ALLOC);
    if (nngRes != 0)
    {
        logger.error(fmt::format("MDApi::SendSubTickReq,nng_send {}; res {}; msg {}", rpcUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    }  
    // nng_send with NNG_FLAG_ALLOC, nng_free not needed
    // nng_free(reqBuf, req.byteSize);
    logger.debug("MDApi::SendSubTickReq,{}", req.DebugInfo());
    
    RPCRspHeader* buf = nullptr;
    size_t sz;
    nngRes = nng_recv(rpcSock, reinterpret_cast<void*>(&buf), &sz, NNG_FLAG_ALLOC);
    if (nngRes != 0)
    {
        logger.error(fmt::format("MDApi::SendSubTickReq,SubTick nng_recv {}; res {}; msg {}", rpcUrl, nngRes, nng_strerror(nngRes)));
        exit(-1);
    } 
    if (buf->rpc != RPCType::SubTick)
    {
        logger.error("MDApi::SendSubTickReq,nng_recv received req {}; but not SubTickReq", buf->DebugInfo());
    }
    SubTickRsp* rsp = reinterpret_cast<SubTickRsp*>(buf);
    logger.info("MDApi::SendSubTickReq,SubTickRsp received; {}", rsp->DebugInfo());
    nng_free(buf, sz);   
    
    OnSubTickRsp(rsp);

    logger.info("MDApi::SendSubTickReq,OnSubTickRsp called;");

     
    return rpcID;
}

void MDApi::HandleEvent()
{
    while (true)
    {
        EventHeader* buf = nullptr;
        size_t sz;
        int nngRes;
        nngRes = nng_recv(eventSock, &buf, &sz, NNG_FLAG_ALLOC);
        if (nngRes != 0)
        {
            logger.error("MDApi::HandleEvent,nng_recv {}; res {}; msg {}", eventUrl, nngRes, nng_strerror(nngRes));
            exit(-1);
        }
        logger.debug("MDApi::HandleEvent,receive event {}", buf->DebugInfo());
        switch (buf->event)
        {
            case EventType::EventMDReady:
            {
                const MDReady* event = reinterpret_cast<MDReady*>(buf);
                OnMDReady(event);
                logger.debug("MDApi::HandleEvent,OnMDReady called");
                break;
            }
            case EventType::EventTDReady:
            {
                const TDReady* event = reinterpret_cast<TDReady*>(buf);
                break;
            }
            case EventType::EventTick:
            {
                const Tick* event = reinterpret_cast<Tick*>(buf);
                OnTick(event);
                logger.debug("MDApi::HandleEvent,OnTick called");
                break;
            }
            case EventType::EventBar:
            {
                const Bar* event = reinterpret_cast<Bar*>(buf);
                break;
            }
            case EventType::EventOrder:
            {
                const Order* event = reinterpret_cast<Order*>(buf);
                break;
            }
            case EventType::EventTrade:
            {
                const Trade* event = reinterpret_cast<Trade*>(buf);
                break;
            }    
            default:
            {
                logger.error("MDApi::HandleEvent,unsupported event,{}", buf->DebugInfo());
                exit(-1);
            }
        }
        nng_free(buf, sz);
    }
}

};
