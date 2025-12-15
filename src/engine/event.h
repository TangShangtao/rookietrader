#pragma once
#include <any>
#include <functional>
#include <vector>
#include <concurrentqueue.h>

namespace rk::event
{
    enum class EventType: uint8_t
    {
        EVENT_MD_DISCONNECTED,
        EVENT_TD_DISCONNECTED,
        EVENT_TICK_DATA,
        EVENT_TRADE_DATA,
        EVENT_CANCEL_DATA,
        EVENT_ORDER_ERROR,
        EVENT_ALGO_REQ,
        UNKNOWN
    };
    struct Event
    {
        EventType   event_type = EventType::UNKNOWN;
        std::any    data;
        Event() = default;
        Event(EventType event_type, std::any&& data): event_type(event_type), data(std::move(data)) {}
    };
    class EventLoop
    {
    public:
        EventLoop()
        {
            _event_handlers.resize(static_cast<uint8_t>(EventType::UNKNOWN));
        }
        ~EventLoop() = default;
        void register_handler(EventType event_type, const std::function<void(std::any&)>& event_handler)
        {
            _event_handlers[static_cast<uint8_t>(event_type)].push_back(event_handler);
        }
        void push_event(EventType event_type, std::any&& data)
        {
            _mpsc_queue.enqueue(Event(event_type, std::move(data)));
        }
        void handle_event()
        {
            Event event;
            bool updated = _mpsc_queue.try_dequeue(event);
            if (updated)
            {
                for (
                    const auto& event_handler :
                    _event_handlers[static_cast<uint8_t>(event.event_type)]
                )
                {
                    event_handler(event.data);
                }
            }

        }

    private:
        std::vector<std::vector<std::function<void(std::any&)>>> _event_handlers;
        moodycamel::ConcurrentQueue<Event> _mpsc_queue;
    };
};