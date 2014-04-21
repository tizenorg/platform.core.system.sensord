cmake_minimum_required(VERSION 2.6)
project(sensord CXX)

#add_definitions(-Wall -g -DUSE_FILE_DEBUG)
#add_definitions(-Wall -g -D_DEBUG)
#add_definitions(-Wall -g -pg)
add_definitions(-Wall -std=gnu++0x)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/libsensord)

SET(SERVER_SRCS
	server.cpp
	command_worker.cpp
	main.cpp
)

add_executable(${PROJECT_NAME} ${SERVER_SRCS})

target_link_libraries(${PROJECT_NAME} ${rpkgs_LDFLAGS} "sensord-server")
set(CMAKE_CXX_FLAGS "-lrt -ldl -pthread ${CMAKE_CXX_FLAGS}")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
