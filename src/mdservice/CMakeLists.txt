cmake_minimum_required(VERSION 3.15)

set(src 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/mdservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/implements/mdctp.cpp
)

add_executable(MDService ${src})
target_link_directories(MDService PRIVATE ${root}/thirdparty/lib/CTP6.3.15)
target_link_libraries(MDService thostmduserapi_se nng rookietrader)

