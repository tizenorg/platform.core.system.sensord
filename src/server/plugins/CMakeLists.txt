cmake_minimum_required(VERSION 2.6)
project(sensord-plugins CXX)

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(plugin_pkgs REQUIRED dlog libxml-2.0)

FOREACH(flag ${plugin_pkgs_LDFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

FOREACH(flag ${plugin_pkgs_CFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/server)
include_directories(${CMAKE_SOURCE_DIR}/src/shared)

IF("${AUTO_ROTATION}" STREQUAL "ON")
list (APPEND DIR_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/auto_rotation")
list (APPEND PLUGIN_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/auto_rotation/auto_rotation_alg.cpp")
list (APPEND PLUGIN_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/auto_rotation/auto_rotation_alg_emul.cpp")
list (APPEND PLUGIN_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/auto_rotation/auto_rotation_sensor.cpp")
list (APPEND PLUGIN_DEFS "-DENABLE_AUTO_ROTATION")
ENDIF()


set(PLUGIN_SRCS ${PLUGIN_SRCS} PARENT_SCOPE)
set(DIR_INCLUDE ${DIR_INCLUDE} PARENT_SCOPE)
set(PLUGIN_DEFS ${PLUGIN_DEFS} PARENT_SCOPE)

