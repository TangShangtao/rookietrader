#pragma once
#include <string>
#include <unordered_set>
#include <thread>
#include "protocol.h"

namespace rookietrader
{

class Client
{
public:
    Client(const std::string& event_url, const std::string& rpc_url)
        : event_url(event_url), rpc_url(rpc_url)
    {

    }
    virtual ~Client() = default;

    void SubscribeEvent(EventType event)
    {
        subscribed_events.insert(event);
    }

    void StartListen()
    {

    }

    void StopListen()
    {

    }

    virtual Event* ReceiveOneEvent() = 0;
private:
    void HandleEvent()
    {
        while (true)
        {
            Event* event_data_ptr = nullptr;
            event_data_ptr = ReceiveOneEvent();
            
        }
    }

protected:
    std::string event_url;
    std::string rpc_url;
    std::unordered_set<EventType> subscribed_events;
private:
};

};

