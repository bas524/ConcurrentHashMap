cmake_minimum_required(VERSION 3.12)
project(ConcurrentHashMap)

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/include)
include_directories(.)
link_directories(/usr/local/lib)
add_executable(ConcurrentHashMap main.cpp ConcurrentHashMap.h)
target_link_libraries(ConcurrentHashMap pthread debug PocoFoundationd optimized PocoFoundation)