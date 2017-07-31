cmake_minimum_required( VERSION 3.1 )
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} )
include ("cmake/myHunter.cmake")

# build systems
option(USE_CONAN OFF)
option(USE_HUNTER OFF)

#build targets
option(BUILD_GUI "Build GUI" ON)	
option(BUILD_TESTS "Build Tests" ON)

project( KinectFaker )
# Initialize Conan #############################################################
if (USE_CONAN)
    INCLUDE(conanbuildinfo.cmake)
    CONAN_BASIC_SETUP()
endif()

# Find Protobuf
myhunter_add_package(Protobuf)
find_package( Protobuf REQUIRED )

# protobuf lib
protobuf_generate_cpp(PROTO_SRC PROTO_HDR "KinectFileDef.proto")
add_library(protobuf_def ${PROTO_SRC} "${PROTO_HDR};KinectFileDef.proto")
target_include_directories(protobuf_def PUBLIC ${PROTOBUF_INCLUDE_DIR})
target_link_libraries(protobuf_def PUBLIC ${Protobuf_LIBRARIES})

# Faker library
add_subdirectory("Proxy")   

## (optional) Player - plays ksk files
if(BUILD_GUI)
	add_subdirectory("GUI")
endif(BUILD_GUI)

if (BUILD_TESTS)
	SET(CTEST_OUTPUT_ON_FAILURE ON)
	enable_testing()
	add_subdirectory("tests")
endif()

if (BUILD_Player)
	set_property( DIRECTORY PROPERTY 
		VS_STARTUP_PROJECT "GUI"
		)
else()
endif()