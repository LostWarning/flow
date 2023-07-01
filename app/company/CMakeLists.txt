cmake_minimum_required(VERSION 3.9.1)

project(flow VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)


set(SOURCES src/main.cpp)
link_libraries(websockets)
add_executable(company-service ${SOURCES})
target_include_directories(company-service PRIVATE common)