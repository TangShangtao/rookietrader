file(GLOB_RECURSE src
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${pub_src}
)
add_library(rk_engine SHARED ${src})
target_include_directories(rk_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rk_engine PUBLIC ${3rdparty_so} rk_adapter)
target_link_options(rk_engine PRIVATE "-Wl,--as-needed,-fPIC")

