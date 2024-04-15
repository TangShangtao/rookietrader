cmake_minimum_required(VERSION 3.15)

set(mdCTPSource 
    main.cpp
    mdservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/implements/mdCTP.cpp
)

add_executable(mdCTP ${mdCTPSource})
target_link_directories(mdCTP PRIVATE ${root}/lib/thirdparty/CTP6.3.15)
target_link_libraries(mdCTP thostmduserapi_se nng)

