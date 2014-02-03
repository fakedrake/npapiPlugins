#/**********************************************************\ 
# Auto-generated Mac project definition file for the
# Codebender.cc project
#\**********************************************************/

# Mac template platform definition CMake file
# Included from ../CMakeLists.txt

include_directories ( /Developer/Headers/FlatCarbon )

find_library(SECURITY_FRAMEWORK Security)
find_path(SECURITY_INCLUDE_DIR Security/Security.h )
include_directories (${SECURITY_INCLUDE_DIR})

find_library(SERIAL Serial)
find_path(SECURITY_INCLUDE_DIR serial/include/Serial.h )
include_directories (${SERIAL_INCLUDE_DIR})

# remember that the current source dir is the project root; this file is in Mac/
file (GLOB PLATFORM RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    Mac/[^.]*.cpp
    Mac/[^.]*.h
    Mac/[^.]*.cmake
    )

# use this to add preprocessor definitions
add_definitions(
    
)


SOURCE_GROUP(Mac FILES ${PLATFORM})

set (SOURCES
    ${SOURCES}
    ${PLATFORM}
    )

set(PLIST "Mac/bundle_template/Info.plist")
set(STRINGS "Mac/bundle_template/InfoPlist.strings")
set(LOCALIZED "Mac/bundle_template/Localized.r")

add_mac_plugin(${PROJECT_NAME} ${PLIST} ${STRINGS} ${LOCALIZED} SOURCES)

# add library dependencies here; leave ${PLUGIN_INTERNAL_DEPS} there unless you know what you're doing!
target_link_libraries(${PROJECT_NAME}
    ${PLUGIN_INTERNAL_DEPS}
		"-framework Security"
    )

