cmake_minimum_required(VERSION 2.6)
project(sensord CXX)

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(server_pkgs REQUIRED glib-2.0 gio-2.0 dlog libsystemd-daemon libxml-2.0 cynara-client cynara-creds-socket cynara-session)

add_subdirectory(plugins)

add_definitions(${PLUGIN_DEFS})

configure_file(sensor_loader.cpp.in sensor_loader.cpp)

INCLUDE_DIRECTORIES(include)
FILE(GLOB SERVER_SRCS *.c *.cpp)

FOREACH(flag ${server_pkgs_LDFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

FOREACH(flag ${server_pkgs_CFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/client)
include_directories(${CMAKE_SOURCE_DIR}/src/shared)
include_directories(${DIR_INCLUDE})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt -ldl -pthread -fPIE")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

add_executable(${PROJECT_NAME} ${SERVER_SRCS} ${PLUGIN_SRCS})

target_link_libraries(${PROJECT_NAME} ${lib_pkgs_LDFLAGS} "sensord-shared")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
