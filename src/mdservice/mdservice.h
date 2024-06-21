// connect to different fronts and publish marketdata
#pragma once
#include "protocol.h"
#include "tools/logger.h"
#include "nng/nng.h"
#include "nng/protocol/pubsub0/pub.h"
#include "nng/protocol/reqrep0/rep.h"
#include "nlohmann/json.hpp"

#include <thread>
#include <memory>
#include <atomic>
namespace rookietrader
{

class MDService
{
public:
    // set communicate url, read basic config from config.json
    // explicit MDService(const std::string& configPath);
    MDService(
        const std::string& eventUrl, 
        const std::string& rpcUrl, 
        const std::string& accountID, 
        const std::string& password,
        const std::string& frontAddr,
        const std::string& loggerName,
        const std::string& logMode
    );
    virtual ~MDService();
    void Run();
    void Stop();
private:
    // connect and login to broker's marketdata front
    virtual bool OnPrepareMDReq(const PrepareMDReq* req) = 0;
    // subscribe market data of many instruments calling api only once
    virtual bool OnSubTickReq(const SubTickReq* subTickReq) = 0;
    // dissconnect(proactive) from broker's marketdata front
    virtual bool OnDisconnect() = 0;
protected:
    // void WaitPrepareMDReq();
    // void WaitSubTickReq();
    void WaitReq();
    // rsp PrepareMD
    void SendPrepareMDRsp(bool isError, const std::string& msg);
    // rsp SubTick
    void SendSubTickRsp(bool isSucc, const std::string& msg);
    // generate MDReady event, publish to subscribers
    void PublishMDReady();
    // generate Tick event, publish to subscribers
    void PublishTick(const Tick& tick);

private:
    void HandleReq();

protected:
    // communication url
    const std::string eventUrl;
    const std::string rpcUrl;
    // accountID given by broker
    const std::string accountID;
    // password given by broker
    const std::string password;
    // ip addr of broker's marketdata front
    const std::string frontAddr;
    // log api
    const Logger logger;
    // rpc req id
    uint32_t prepareMDRpcID = 0;
    uint32_t subTickRpcID = 0;
    bool frontConnected = false;
private:
    nng_socket eventSock;
    nng_socket rpcSock;
    int nngRes;
    std::shared_ptr<std::thread> handleReqThread;
    std::atomic_bool running;

    
};

};
