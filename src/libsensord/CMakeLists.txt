cmake_minimum_required(VERSION 2.6)
project(sensor CXX)

# to install pkgconfig setup file.
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(VERSION_MAJOR 1)
SET(VERSION "${VERSION_MAJOR}.1.0")

include(FindPkgConfig)
pkg_check_modules(rpkgs REQUIRED vconf glib-2.0)
add_definitions(${rpkgs_CFLAGS})

#add_definitions(-Wall -O3 -omit-frame-pointer -lm)
#add_definitions(-Wall -g -lma -DUSE_FILE_DEBUG)
#add_definitions(-D_DEBUG)
add_definitions(-Wall -g -lma -fPIC -DUSE_DLOG_LOG -std=c++0x)
#add_definitions(-fvisibility=hidden -lm -DUSE_DLOG_LOG)
#add_definitions(-fvisibility=hidden -lm)

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
#        ADD_DEFINITIONS("-DTARGET -DHWREV_CHECK -DUSE_MPU3050_GYRO")
	ADD_DEFINITIONS("-DTARGET -DHWREV_CHECK")
        MESSAGE("add -DTARGET")
ELSE("${ARCH}" STREQUAL "arm")
        ADD_DEFINITIONS("-DSIMULATOR")
        MESSAGE("add -DSIMULATOR")
ENDIF("${ARCH}" STREQUAL "arm")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} SHARED
	client.cpp
	csensor_event_listener.cpp
	sensor_info_manager.cpp
	csensor_handle_info.cpp
	client_common.cpp
	command_channel.cpp
	poller.cpp
)

target_link_libraries(${PROJECT_NAME} ${rpkgs_LDFLAGS} ${GLES_LDFLAGS} "sensord-share")
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES VERSION ${VERSION})

configure_file(${PROJECT_NAME}.pc.in ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc @ONLY)

#install(DIRECTORY include/ DESTINATION ${INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
install(TARGETS ${PROJECT_NAME} DESTINATION ${LIBDIR} COMPONENT RuntimeLibraries)

install(FILES sensor.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_internal_deprecated.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_internal.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES poller.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES creg_event_info.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES csensor_event_listener.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_info_manager.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES csensor_handle_info.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES client_common.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_accel.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_geomag.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_light.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_proxi.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_gyro.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_pressure.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_context.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_gravity.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_linear_accel.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_orientation.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_rv.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES sensor_temperature.h DESTINATION ${INCLUDEDIR}/sensor/)
install(FILES ${PROJECT_NAME}.pc DESTINATION ${LIBDIR}/pkgconfig)
