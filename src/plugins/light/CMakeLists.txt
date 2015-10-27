cmake_minimum_required(VERSION 2.6)
project(light CXX)

SET(SENSOR_NAME light_sensor)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/client)

FOREACH(flag ${light_pkgs_LDFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

FOREACH(flag ${light_pkgs_CFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

add_library(${SENSOR_NAME} SHARED
		light_sensor.cpp
		)

target_link_libraries(${SENSOR_NAME} ${light_pkgs_LDFLAGS} "-lm")

install(TARGETS ${SENSOR_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/sensord)
