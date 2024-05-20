// rpc api to mdserice.
// use mdapi to subscribe marketdata from mdservice.
#pragma once
#include "protocol.h"
#include "tools/logger.h"
#include "nng/nng.h"
#include "nlohmann/json.hpp"
#include <string>
#include <vector>

namespace rookietrader
{
class MDApi
{
public:
    // establish connection with service
    explicit MDApi(const std::string& configPath);
    virtual ~MDApi();
    // call Init to start receiving events, call it at end of main thread
    void Init();
    // cannot bind Join and use Join in Python, use time.sleep! why?
    void Join();
    // send prepareMD req(sync)
    int SendPrepareMDReq();
    // send subTick req(sync)
    int SendSubTickReq(ExchangeID exchange, std::vector<std::string>& instruments);
    
    // Event Callback
    virtual void OnMDReady(const MDReady* event) = 0;
    virtual void OnTick(const Tick* event) = 0;
    // Rpc Callback
    virtual void OnPrepareMDRsp(const PrepareMDRsp* rsp) = 0;
    virtual void OnSubTickRsp(const SubTickRsp* rsp) = 0;    

private:
    
    void HandleEvent();
private:
    // communication url
    std::string eventUrl;
    std::string rpcUrl;
    // log api
    Logger logger;
    // rpcID return by MDApi
    int rpcID = 0;

    nng_socket eventSock;
    nng_socket rpcSock;
    std::shared_ptr<std::thread> handleRspThread;
    std::shared_ptr<std::thread> handleEventThread;

};

};

