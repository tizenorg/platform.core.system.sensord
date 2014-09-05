CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(sensor-tc C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")
SET(VERSION 1.0)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED glib-2.0 dlog)

add_definitions(${rpkgs_CFLAGS})
add_definitions(-DPREFIX="${CMAKE_INSTALL_PREFIX}")

configure_file(${PROJECT_NAME}.pc.in ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc @ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/libsensord)
include_directories(${CMAKE_SOURCE_DIR}/src/shared)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

add_executable(accelerometer src/accelerometer.c)
add_executable(geomagnetic src/geomagnetic.c)
add_executable(orientation src/orientation.c)

SET_TARGET_PROPERTIES(accelerometer PROPERTIES LINKER_LANGUAGE C)
SET_TARGET_PROPERTIES(geomagnetic PROPERTIES LINKER_LANGUAGE C)
SET_TARGET_PROPERTIES(orientation PROPERTIES LINKER_LANGUAGE C)

link_directories(../src/libsensord/)

target_link_libraries(accelerometer glib-2.0 dlog sensor)
target_link_libraries(geomagnetic glib-2.0 dlog sensor)
target_link_libraries(orientation glib-2.0 dlog sensor)

INSTALL(TARGETS accelerometer DESTINATION /usr/bin/)
INSTALL(TARGETS geomagnetic DESTINATION /usr/bin/)
INSTALL(TARGETS orientation DESTINATION /usr/bin/)



