file(GLOB_RECURSE src
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
add_library(rk_adapter SHARED ${src} ${pub_src})
target_include_directories(rk_adapter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rk_adapter PUBLIC ${3rdparty_so})