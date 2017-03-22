find_package(GTest REQUIRED)
include_directories("${PROJECT_SOURCE_DIR}/src/net")
include_directories("${PROJECT_SOURCE_DIR}/src/nodes")

add_executable(BaseAnimationTst TestAnimation.cpp BaseAnimationTest.cpp)
target_link_libraries(BaseAnimationTst ${GTEST_BOTH_LIBRARIES})
target_link_libraries(BaseAnimationTst WedLitLib)

configure_file(
	animctest1.json "${CMAKE_CURRENT_BINARY_DIR}/animctest1.json" COPYONLY
	)
add_executable(AnimationControllerTst AnimationControllerTest.cpp)
target_link_libraries(AnimationControllerTst WedLitLib pthread)
target_link_libraries(AnimationControllerTst ${GTEST_BOTH_LIBRARIES})
add_test(BaseAnimationTest1 BaseAnimationTst)
add_test(AnimationControllerTest AnimationControllerTst)

#add_executable(Discovery discovery.cpp)
#target_link_libraries(Discovery Network Nodes pthread)

add_executable(KeyframeDataTest TestAnimation.cpp KeyframeDataTest.cpp)
target_link_libraries(KeyframeDataTest WedLitLib ${GTEST_BOTH_LIBRARIES})
add_test(KeyframeDataTest KeyframeDataTest)

if (TEST_VERBOSE)
	add_definitions(-DTEST_VERBOSE)
endif(TEST_VERBOSE)