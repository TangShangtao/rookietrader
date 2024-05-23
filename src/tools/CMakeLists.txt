cmake_minimum_required(VERSION 3.15)

set(src 
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/database.cpp 
)

message(${src})
target_sources(rookietrader PRIVATE ${src})
target_link_libraries(rookietrader pq pqxx)