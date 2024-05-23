# librookietrader.a
# 
add_library(rookietrader STATIC)


# mdservice: executable to receive md
add_subdirectory(mdservice)
add_subdirectory(test)



add_subdirectory(tools)
add_subdirectory(api)
add_subdirectory(binding)