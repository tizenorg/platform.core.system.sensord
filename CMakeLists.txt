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

# Internal Debugging Options
#add_definitions(-Wall -g -D_DEBUG)

# For Emulator
FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" MATCHES "^arm.*")
        ADD_DEFINITIONS("-DTARGET")
        MESSAGE("add -DTARGET")
ELSE("${ARCH}" MATCHES "^arm.*")
        ADD_DEFINITIONS("-DSIMULATOR")
        MESSAGE("add -DSIMULATOR")
ENDIF("${ARCH}" MATCHES "^arm.*")

# Installing files
CONFIGURE_FILE(sensor_plugins.xml.in sensor_plugins.xml @ONLY)
CONFIGURE_FILE(sensors.xml.in sensors.xml @ONLY)

INSTALL(FILES sensor_plugins.xml sensors.xml DESTINATION etc)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.APLv2 DESTINATION share/license RENAME sensord)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.APLv2 DESTINATION share/license RENAME libsensord)

IF("${TEST_SUITE}" STREQUAL "ON")
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.APLv2 DESTINATION share/license RENAME test)
ENDIF()

# Sub-directory
add_subdirectory(systemd)
add_subdirectory(src)

IF("${TEST_SUITE}" STREQUAL "ON")
add_subdirectory(test)
ENDIF()

