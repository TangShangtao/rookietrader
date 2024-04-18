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
    explicit MDApi(const nlohmann::json& config);
    ~MDApi();
    void RegisterSpi(MDSpi* spi);
    int SendPrepareMDReq();
    int SendSubTickReq(ExchangeID exchange, std::vector<std::string>& instruments);
    

private:
    void HandleRsp();
    void HandleEvent();
private:
    // communication url
    const std::string eventUrl;
    const std::string rpcUrl;
    // log api
    const Logger logger;
    // rpcID return by MDApi
    int rpcID = 0;
    // rpc req id
    uint32_t prepareMDRpcID = 0;
    uint32_t subTickRpcID = 0;
    // MDSpi registered by user
    MDSpi* spi;

    nng_socket eventSock;
    nng_socket rpcSock;
    int nngRes;
    std::shared_ptr<std::thread> handleRspThread;
    std::shared_ptr<std::thread> handleEventThread;

};

};

