CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sensord CXX)
INCLUDE(GNUInstallDirs)

# Common Options
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -omit-frame-pointer -std=gnu++0x")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-section -Wl,--print-gc-section")
MESSAGE("FLAGS: ${CMAKE_CXX_FLAGS}")
MESSAGE("FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

# Internal Debugging Options
#add_definitions(-Wall -g -D_DEBUG)

# Sub-directory
ADD_SUBDIRECTORY(systemd)
ADD_SUBDIRECTORY(src)

IF("${TEST_SUITE}" STREQUAL "ON")
ADD_SUBDIRECTORY(test)
ENDIF()

