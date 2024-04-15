cmake_minimum_required(VERSION 3.15)

set(src 
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp 
)

message(${src})
add_library(rookietrader ${src})
# target_link_directories(rookietrader PRIVATE ${root}/thirdparty/lib/CTP6.3.15)
# target_link_libraries(mdCTP thostmduserapi_se nng)