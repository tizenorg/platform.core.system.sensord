cmake_minimum_required(VERSION 2.6)
project(sensord CXX)

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(server_pkgs REQUIRED glib-2.0 gio-2.0 dlog libsystemd-daemon libxml-2.0 cynara-client cynara-creds-socket cynara-session)
FOREACH(flag ${server_pkgs_LDFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

FOREACH(flag ${server_pkgs_CFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/client)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt -ldl -pthread -fPIE")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

SET(SERVER_SRCS
	dbus_util.cpp
	server.cpp
	command_worker.cpp
	permission_checker.cpp
	main.cpp

)

add_library(sensord-server SHARED
	worker_thread.cpp
	cvirtual_sensor_config.cpp
	csensor_event_queue.cpp
	csensor_event_dispatcher.cpp
	csensor_usage.cpp
	cplugin_info_list.cpp
	sensor_plugin_loader.cpp
	sensor_base.cpp
	physical_sensor.cpp
	virtual_sensor.cpp
	cclient_info_manager.cpp
	cclient_sensor_record.cpp
	cconfig.cpp
)
target_link_libraries(sensord-server ${shared_pkgs_LDFLAGS} "-lrt -ldl -pthread" "sensord-share")
configure_file(sensord-server.pc.in ${CMAKE_CURRENT_SOURCE_DIR}/sensord-server.pc @ONLY)
install(TARGETS sensord-server DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES sensord-server.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

add_executable(${PROJECT_NAME} ${SERVER_SRCS})

target_link_libraries(${PROJECT_NAME} ${server_pkgs_LDFLAGS} "sensord-server")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)



IF("${ACCEL}" STREQUAL "ON")
add_subdirectory(plugins/accel)
ENDIF()
IF("${GYRO}" STREQUAL "ON")
add_subdirectory(plugins/gyro)
ENDIF()
IF("${PROXI}" STREQUAL "ON")
add_subdirectory(plugins/proxi)
ENDIF()
IF("${LIGHT}" STREQUAL "ON")
add_subdirectory(plugins/light)
ENDIF()
IF("${GEO}" STREQUAL "ON")
add_subdirectory(plugins/geo)
ENDIF()
IF("${AUTO_ROTATION}" STREQUAL "ON")
add_subdirectory(plugins/auto_rotation)
ENDIF()
IF("${PRESSURE}" STREQUAL "ON")
add_subdirectory(plugins/pressure)
ENDIF()
IF("${TEMPERATURE}" STREQUAL "ON")
add_subdirectory(plugins/temperature)
ENDIF()
IF("${HUMIDITY}" STREQUAL "ON")
add_subdirectory(humidity)
ENDIF()
IF("${ULTRAVIOLET}" STREQUAL "ON")
add_subdirectory(plugins/ultraviolet)
ENDIF()
IF("${BIO_LED_RED}" STREQUAL "ON")
add_subdirectory(plugins/bio_led_red)
ENDIF()
IF("${ORIENTATION}" STREQUAL "ON")
set(SENSOR_FUSION_ENABLE "1")
set(ORIENTATION_ENABLE "1")
ENDIF()
IF("${GEOMAGNETIC_RV}" STREQUAL "ON")
set(SENSOR_FUSION_ENABLE "1")
ENDIF()
IF("${GAMING_RV}" STREQUAL "ON")
set(SENSOR_FUSION_ENABLE "1")
ENDIF()
IF("${TILT}" STREQUAL "ON")
set(SENSOR_FUSION_ENABLE "1")
set(TILT_ENABLE "1")
ENDIF()
IF("${UNCAL_GYRO}" STREQUAL "ON")
set(SENSOR_FUSION_ENABLE "1")
set(UNCAL_GYRO_ENABLE "1")
ENDIF()
IF("${GRAVITY}" STREQUAL "ON")
set(SENSOR_FUSION_ENABLE "1")
set(ORIENTATION_ENABLE "1")
set(GRAVITY_ENABLE "1")
ENDIF()
IF("${LINEAR_ACCEL}" STREQUAL "ON")
set(SENSOR_FUSION_ENABLE "1")
set(ORIENTATION_ENABLE "1")
set(GRAVITY_ENABLE "1")
set(LINEAR_ACCELERATION_ENABLE "1")
ENDIF()
IF("${SENSOR_FUSION_ENABLE}" STREQUAL "1")
add_subdirectory(sensor_fusion)
add_subdirectory(plugins/fusion)
ENDIF()
IF("${ORIENTATION_ENABLE}" STREQUAL "1")
add_subdirectory(plugins/orientation)
ENDIF()
IF("${GRAVITY_ENABLE}" STREQUAL "1")
add_subdirectory(plugins/gravity)
ENDIF()
IF("${LINEAR_ACCELERATION_ENABLE}" STREQUAL "1")
add_subdirectory(plugins/linear_accel)
ENDIF()
IF("${TILT_ENABLE}" STREQUAL "1")
add_subdirectory(plugins/tilt)
ENDIF()
IF("${UNCAL_GYRO_ENABLE}" STREQUAL "1")
add_subdirectory(plugins/uncal_gyro)
ENDIF()

add_subdirectory(plugins/rotation_vector)
