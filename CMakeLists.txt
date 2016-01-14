cmake_minimum_required(VERSION 3.2)
project(client)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lssl -lcrypto -std=c++11 ")

set(SOURCE_FILES main.cpp message.h message.cpp)

add_executable(client ${SOURCE_FILES})

target_link_libraries(client ssl crypto pthread)