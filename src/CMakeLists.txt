add_subdirectory(animations)
add_subdirectory(audio)
add_subdirectory(net)
add_subdirectory(nodes)
add_subdirectory(tests)

add_library(WedLitLib STATIC 
	BaseAnimation.cpp 
	AnimationController.cpp 
	Server.cpp
	Connection.cpp
	Commands.cpp)
target_link_libraries(WedLitLib Qt5::Core Qt5::Multimedia)
target_link_libraries(WedLitLib Animations Audio Nodes Network)

add_executable(WedLit WedLit.cpp)
target_link_libraries(WedLit WedLitLib pthread)

install (TARGETS WedLit DESTINATION bin)