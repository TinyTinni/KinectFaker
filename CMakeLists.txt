cmake_minimum_required( VERSION 3.1 )
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} )
project( KinectFaker )

# Find Kinect SDK v1
find_package( KinectSDK REQUIRED )
find_package( Protobuf REQUIRED )

set(SUBHOOK_NO_TESTS ON)
set(SUBHOOK_STATIC ON)
add_subdirectory("Faker/subhook")

# protobuf lib
protobuf_generate_cpp(PROTO_SRC PROTO_HDR "KinectFileDef.proto")
add_library(protobuf_def ${PROTO_SRC} "${PROTO_HDR};KinectFileDef.proto")
target_include_directories(protobuf_def PUBLIC ${PROTOBUF_INCLUDE_DIR})
target_link_libraries(protobuf_def PUBLIC protobuf::libprotobuf)

# Faker library
add_subdirectory("Faker")

## Recorder
add_subdirectory("Recorder")	
set_property( DIRECTORY PROPERTY 
	VS_STARTUP_PROJECT "Recorder"
	)

## (optional) Player - plays ksk files
option (BUILD_PLAYER ON)	
if(BUILD_PLAYER)
	add_subdirectory("Player")
endif(BUILD_PLAYER)