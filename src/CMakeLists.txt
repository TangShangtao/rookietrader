add_subdirectory(mdservice)
add_subdirectory(test)

add_library(rookietrader STATIC)
target_link_libraries(rookietrader nng)

add_subdirectory(tools)
add_subdirectory(api)