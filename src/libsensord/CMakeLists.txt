cmake_minimum_required(VERSION 2.6)
project(sensor CXX)

SET(VERSION_MAJOR 1)
SET(VERSION "${VERSION_MAJOR}.1.0")

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(lib_pkgs REQUIRED vconf glib-2.0)

FOREACH(flag ${lib_pkgs_LDFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

FOREACH(flag ${lib_pkgs_CFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -g -fPIC")

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

target_link_libraries(${PROJECT_NAME} ${lib_pkgs_LDFLAGS} "sensord-share")
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES VERSION ${VERSION})

configure_file(${PROJECT_NAME}.pc.in ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc @ONLY)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT RuntimeLibraries)
install(FILES sensor_internal_deprecated.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_internal.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES poller.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES creg_event_info.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES csensor_event_listener.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_info_manager.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES csensor_handle_info.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES client_common.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_accel.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_geomag.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_light.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_proxi.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_gyro.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_pressure.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_context.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_auto_rotation.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_gravity.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_linear_accel.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_orientation.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_rv.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_geomagnetic_rv.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_temperature.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_motion.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_deprecated.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES ${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
