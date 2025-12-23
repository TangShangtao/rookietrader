#include <iox2/iceoryx2.hpp>
#include <cstdint>
#include <iostream>
#include <utility>
#include "data_type.h"
#include "serialization.h"
#include "util/datetime.h"
#include <array>
#include <magic_enum/magic_enum.hpp>
#include <format>


constexpr iox2::bb::Duration CYCLE_TIME = iox2::bb::Duration::from_secs(1);
using namespace rk::data_type;
int main(int argc, char *argv[])
{
    std::string mode = argv[1];
    iox2::set_log_level_from_env_or(iox2::LogLevel::Info);
    auto node = iox2::NodeBuilder().create<iox2::ServiceType::Ipc>().value();
    auto service = node.service_builder(iox2::ServiceName::create("My/Funk/ServiceName").value())
        .publish_subscribe<TickData>()
        .open_or_create()
        .value();
    if (mode == "publisher")
    {
        auto publisher = service.publisher_builder().create().value();
        auto counter = 0;
        while (node.wait(CYCLE_TIME).has_value())
        {
            counter += 1;
            auto sample = publisher.loan_uninit().value();
            auto initialized_sample =
                sample.write_payload(
                    TickData{
                        { std::format("test{}", counter).c_str(), "", Exchange::UNKNOWN, ProductClass::UNKNOWN},
                        0,
                        rk::util::DateTime::now(),
                        0,0,0,0,
                        {{{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}}},
                        {{{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}}}
                    }
                ); // NOLINT
            iox2::send(std::move(initialized_sample)).value();
            std::cout << "Send sample " << counter << "..." << std::endl;
        }
    }
    if (mode == "subscriber")
    {
        auto subscriber = service.subscriber_builder().create().value();
        std::cout << "Subscriber ready to receive data!" << std::endl;
        while (node.wait(CYCLE_TIME).has_value())
        {
            auto sample = subscriber.receive().value();
            while (sample.has_value())
            {
                std::cout << "received: " << to_json(sample->payload()).dump(4) << std::endl;
                sample = subscriber.receive().value();
            }
        }
    }


    return 0;
}