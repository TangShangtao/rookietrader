#pragma once

#include <zmq_addon.hpp>
namespace rk::ipc
{

    inline zmq::context_t& get_zmq_context_instance()
    {
        static zmq::context_t ctx{1};
        return ctx;
    }

};