#include <cstdint>
#include <iostream>
#include <utility>
#include "data_type.h"
#include "util/datetime.h"
#include <array>
#include <magic_enum/magic_enum.hpp>
#include <format>
#include "util/logger.h"
#include "adapter/adapter.h"
#include "util/ipc.h"
#include <zmq_addon.hpp>
#include <readerwriterqueue.h>
#include <format>
#include "adapter/adapter.h"
#include "engine_impl/engine_impl.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <algorithm>
#include <replxx.hxx>
#include "tools/rk_terminal/util.hpp"
using namespace rk;
using namespace rk::ipc;
using namespace ftxui;
using Replxx = replxx::Replxx;


int main(int argc, char *argv[])
{
    EngineImpl engine{"emt.toml"};
    engine.start_trading();
    engine.algo_insert({
        {"000001.SZ", "000001", data_type::Exchange::SZSE, data_type::ProductClass::STOCK},
        0,
        "TWAP",
        "",
        util::DateTime::now() + util::TimeDelta{{.minutes=2}},
        util::DateTime::now() + util::TimeDelta{{.minutes=3}}
    });

};


