file(GLOB_RECURSE src
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${pub_src}
)
add_executable(md_gateway ${src})
target_include_directories(md_gateway PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(md_gateway PUBLIC ${3rdparty_so} rk_adapter rk_engine)

