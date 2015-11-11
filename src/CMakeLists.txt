cmake_minimum_required(VERSION 2.6)
project(sensord_src CXX)

include_directories(${CMAKE_SOURCE_DIR}/src/shared)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(shared)
