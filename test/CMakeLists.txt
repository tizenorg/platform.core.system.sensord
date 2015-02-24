CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(sensor-tc C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(VERSION 1.0)

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

link_directories(../src/libsensord/)

add_executable(auto_test src/auto_test.c)
add_executable(tc-common src/tc-common.c)

SET_TARGET_PROPERTIES(auto_test PROPERTIES LINKER_LANGUAGE C)
SET_TARGET_PROPERTIES(tc-common PROPERTIES LINKER_LANGUAGE C)

target_link_libraries(auto_test glib-2.0 dlog sensor)
target_link_libraries(tc-common glib-2.0 dlog sensor)

INSTALL(TARGETS auto_test DESTINATION /usr/bin/)
INSTALL(TARGETS tc-common DESTINATION /usr/bin/)
