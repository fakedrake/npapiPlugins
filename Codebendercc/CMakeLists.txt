#/**********************************************************\ 
# 
# Auto-generated CMakeLists.txt for the Codebender.cc project
#
#\**********************************************************/

# Written to work with cmake 2.6
cmake_minimum_required (VERSION 2.6)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.6)


PROJECT(${PLUGIN_NAME})

file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    [^.]*.cc
    [^.]*.cpp
    [^.]*.h
    [^.]*.cmake
    )

# Serial Sources
set(SERIAL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/src/serial.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/include/serial/serial.h
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/include/serial/v8stdint.h
)
if (UNIX)
    # If unix
    list(APPEND SERIAL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/serial/src/impl/unix.cc ${CMAKE_CURRENT_SOURCE_DIR}/serial/include/serial/impl/unix.h)
else ()
    # If windows
    list(APPEND SERIAL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/serial/src/impl/win.cc ${CMAKE_CURRENT_SOURCE_DIR}/serial/include/serial/impl/win.h)
endif()

include_directories(${PLUGIN_INCLUDE_DIRS})

# Generated files are stored in ${GENERATED} by the project configuration
SET_SOURCE_FILES_PROPERTIES(
    ${GENERATED}
    PROPERTIES
    GENERATED 1
    )

SOURCE_GROUP(Generated FILES
    ${GENERATED}
    )

SET( SOURCES
    ${GENERAL}
    ${SERIAL_SRCS}
    ${GENERATED}
	)

# This will include Win/projectDef.cmake, X11/projectDef.cmake, Mac/projectDef 
# depending on the platform
include_platform()
