cmake_minimum_required(VERSION 2.6)
project(sensord-shared CXX)

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
include_directories(${CMAKE_SOURCE_DIR}/src/client)

add_library(${PROJECT_NAME} SHARED
	cbase_lock.cpp
	cmutex.cpp
	cpacket.cpp
	csocket.cpp
	sensor_logs.cpp
	sensor_info.cpp
	poller.cpp
)

target_link_libraries(${PROJECT_NAME} ${shared_pkgs_LDFLAGS} "-lrt -ldl -pthread")

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES
	cpacket.h
	csocket.h
	cbase_lock.h
	sensor_info.h
	poller.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(FILES sensor_deprecated.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_types.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_internal_deprecated.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_internal.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_common.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
install(FILES sensor_hal.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor/)
