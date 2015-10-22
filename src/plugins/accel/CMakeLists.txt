cmake_minimum_required(VERSION 2.6)
project(accel CXX)

SET(SENSOR_NAME accel_sensor)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/libsensord)

FOREACH(flag ${accel_pkgs_LDFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

FOREACH(flag ${accel_pkgs_CFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

add_library(${SENSOR_NAME} SHARED
		accel_sensor.cpp
		)

target_link_libraries(${SENSOR_NAME} ${accel_pkgs_LDFLAGS} "-lm")

install(TARGETS ${SENSOR_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/sensord)
