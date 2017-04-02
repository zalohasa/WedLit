add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/.pioenvs/nodemcuv2/firmware.bin
	COMMAND pio run
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h ${CMAKE_CURRENT_SOURCE_DIR}/src/Keyframes.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/Keyframes.h ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

add_custom_target(
	Firmware ALL
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/.pioenvs/nodemcuv2/firmware.bin
	)

install (
	FILES .pioenvs/nodemcuv2/firmware.bin DESTINATION tools
	)

set_directory_properties(
	PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/.pioenvs/nodemcuv2
	)