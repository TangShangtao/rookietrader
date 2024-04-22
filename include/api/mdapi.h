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
class MDSpi
{
public:
    // Event Callback
    virtual void OnMDReady(const MDReady* event) = 0;
    virtual void OnTick(const Tick* event) = 0;
    // Rpc Callback
    virtual void OnPrepareMDRsp(const PrepareMDRsp* rsp) = 0;
    virtual void OnSubTickRsp(const SubTickRsp* rsp) = 0;
};
class MDApi
{
public:
    // establish connection with service
    explicit MDApi(const nlohmann::json& config);
    ~MDApi();
    // call Init to start receiving rsp and event
    void Init();
    // register callbacks
    void RegisterSpi(MDSpi* spi);
    // send prepareMD req(sync)
    int SendPrepareMDReq();
    // send subTick req(sync)
    int SendSubTickReq(ExchangeID exchange, std::vector<std::string>& instruments);
    

private:
    void HandleEvent();
private:
    // communication url
    const std::string eventUrl;
    const std::string rpcUrl;
    // log api
    const Logger logger;
    // rpcID return by MDApi
    int rpcID = 0;
    // MDSpi registered by user
    MDSpi* spi;

    nng_socket eventSock;
    nng_socket rpcSock;
    std::shared_ptr<std::thread> handleRspThread;
    std::shared_ptr<std::thread> handleEventThread;

};

};

