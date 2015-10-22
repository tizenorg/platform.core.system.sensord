cmake_minimum_required(VERSION 2.6)
project(sf_common CXX)

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(shared_pkgs REQUIRED dlog libxml-2.0)

FOREACH(flag ${shared_pkgs_LDFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

FOREACH(flag ${shared_pkgs_CFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/libsensord)

add_library(sensord-server SHARED
	crw_lock.cpp
	worker_thread.cpp
	cconfig.cpp
	csensor_config.cpp
	cvirtual_sensor_config.cpp
	csensor_event_queue.cpp
	csensor_event_dispatcher.cpp
	csensor_usage.cpp
	cclient_info_manager.cpp
	cclient_sensor_record.cpp
	cplugin_info_list.cpp
	sensor_plugin_loader.cpp
	sensor_hal.cpp
	sensor_base.cpp
	physical_sensor.cpp
	virtual_sensor.cpp
	iio_common.cpp
)

add_library(sensord-share SHARED
	cpacket.cpp
	csocket.cpp
	cbase_lock.cpp
	cmutex.cpp
	common.cpp
	sensor_info.cpp
)

target_link_libraries(sensord-server ${shared_pkgs_LDFLAGS} "-lrt -ldl -pthread" "sensord-share")
target_link_libraries(sensord-share ${shared_pkgs_LDFLAGS} "-lrt -ldl -pthread")
configure_file(sensord-server.pc.in ${CMAKE_CURRENT_SOURCE_DIR}/sensord-server.pc @ONLY)
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc @ONLY)

install(TARGETS sensord-server DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS sensord-share DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES sensord-server.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES ${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES
	crw_lock.h
	worker_thread.h
	cconfig.h
	csensor_config.h
	cvirtual_sensor_config.h
	csensor_event_queue.h
	cplugin_info_list.h
	sensor_plugin_loader.h
	sensor_hal.h
	sensor_base.h
	physical_sensor.h
	virtual_sensor.h
	sf_common.h
	cpacket.h
	csocket.h
	cbase_lock.h
	cmutex.h
	common.h
	sensor_info.h
	iio_common.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(FILES
	sensor_common.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor
)
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
install(FILES sensor_tilt.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_rv.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_geomagnetic_rv.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_gaming_rv.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_temperature.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_motion.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_fusion.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_ultraviolet.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_bio_led_red.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_deprecated.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_uncal_gyro.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_internal_deprecated.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_internal.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)

