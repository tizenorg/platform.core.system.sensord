cmake_minimum_required(VERSION 2.6)
project(sensord_main CXX)
include(GNUInstallDirs)

# Setup For pkgconfig File
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "${PREFIX}/bin")
SET(LIBDIR "${PREFIX}/${CMAKE_INSTALL_LIBDIR}")
SET(INCLUDEDIR "${PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
SET(VERSION 1.0)

# Common Options
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -omit-frame-pointer -std=gnu++0x")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-section -Wl,--print-gc-section")
MESSAGE("FLAGS: ${CMAKE_CXX_FLAGS}")
MESSAGE("FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

add_definitions(-DUSE_DLOG_LOG)
add_definitions(-DLIBDIR="${CMAKE_INSTALL_LIBDIR}")

# Internal Debugging Options
#add_definitions(-Wall -g -D_DEBUG)

# Installing files
CONFIGURE_FILE(sensor_plugins.xml.in sensor_plugins.xml @ONLY)

INSTALL(FILES sensor_plugins.xml DESTINATION etc)
INSTALL(FILES virtual_sensors.xml virtual_sensors.xml DESTINATION etc)

# Sub-directory
add_subdirectory(systemd)
add_subdirectory(src)

IF("${TEST_SUITE}" STREQUAL "ON")
add_subdirectory(test)
ENDIF()

