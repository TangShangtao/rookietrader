// rpc api to mdserice.
// use mdapi to subscribe marketdata from mdservice.
#pragma once
#include "protocol.h"
#include "tools/logger.h"
#include "nng/nng.h"
#include "nlohmann/json.hpp"
#include <string>
#include <vector>
#include <unordered_set>

namespace rookietrader
{
class MDApi
{
public:
    // establish connection with service
    MDApi(
        const std::string& eventUrl,
        const std::string& rpcUrl,
        const std::string& loggerName,
        const std::string& logMode
    );
    explicit MDApi(const std::string& configPath);
    virtual ~MDApi();
    // call Subscribe to subscribe push events
    void Subscribe(EventType event);
    // call Init to start receiving events, call it at end of main thread
    void Init();
    // cannot bind Join and use Join in Python, use time.sleep! why?
    void Join();
    // send prepareMD req(sync)
    PrepareMDRsp SendPrepareMDReq();
    // send subTick req(sync)
    SubTickRsp SendSubTickReq(ExchangeID exchange, std::vector<std::string>& instruments);
    
    // MDApi Event thread start, call reqs in Callbacks
    virtual void OnMDApiStart() = 0;
    // Event Callback
    virtual void OnMDReady(const MDReady* event) = 0;
    virtual void OnTick(const Tick* event) = 0;  

private:
    
    void HandleEvent();
public:
    // log api
    const Logger logger;
    
private:
    // communication url
    const std::string eventUrl;
    const std::string rpcUrl;

    // EventType subscribes
    std::unordered_set<EventType> subcribeEvents;
    // rpcID return by MDApi
    int rpcID = 0;

    nng_socket eventSock;
    nng_socket rpcSock;
    std::shared_ptr<std::thread> handleRspThread;
    std::shared_ptr<std::thread> handleEventThread;

};

};

