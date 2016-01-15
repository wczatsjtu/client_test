cmake_minimum_required(VERSION 3.3)
project(client_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp message.h message.cpp)
add_executable(client_test ${SOURCE_FILES})

target_link_libraries(client_test ssl crypto pthread)