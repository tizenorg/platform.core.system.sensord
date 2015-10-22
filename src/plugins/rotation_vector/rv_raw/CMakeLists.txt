cmake_minimum_required(VERSION 2.6)
project(rv_raw CXX)

SET(SENSOR_NAME rv_raw_sensor)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/libsensord)

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(rv_raw_pkgs REQUIRED vconf)

FOREACH(flag ${rv_raw_pkgs_LDFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

FOREACH(flag ${rv_raw_pkgs_CFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

add_library(${SENSOR_NAME} SHARED
		rv_raw_sensor.cpp
)

target_link_libraries(${SENSOR_NAME} ${rv_raw_pkgs_LDFLAGS} "-lm")

install(TARGETS ${SENSOR_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/sensord)
