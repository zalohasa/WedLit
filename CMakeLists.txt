cmake_minimum_required (VERSION 3.7)
project ("WedLit Controller")

set (WedLit_VERSION_MAJOR 0)
set (WedLit_VERSION_MINOR 1)

set (CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install")

configure_file (
	"${PROJECT_SOURCE_DIR}/inc/config.h.in"
	"${PROJECT_BINARY_DIR}/src/config.h"
	)

find_package(Qt5Core REQUIRED)
#find_package(Qt5Multimedia REQUIRED)

option (TEST_VERBOSE "Make tests print log data to stdout" OFF)

enable_testing()

include_directories("${PROJECT_BINARY_DIR}/src")

include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/src/net")
include_directories("${PROJECT_SOURCE_DIR}/src/audio")
add_subdirectory(src)
add_subdirectory(firmware)

install (DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX})
install (DIRECTORY DESTINATION data/music)
install (
	FILES tools/espota.py DESTINATION tools
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)
install (
	FILES tools/startap.sh DESTINATION tools
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)
install (
	FILES tools/stopap.sh DESTINATION tools
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)

