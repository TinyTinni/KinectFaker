# Example CMakeLists.txt
# FindKInectSDK.cmake copy to CMake\share\cmake-3.5\Modules or same directory as this file

cmake_minimum_required( VERSION 3.1 )
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} )

project( KinectRecorder )

# Find Kinect SDK v1
find_package( KinectSDK REQUIRED )

find_package( Protobuf REQUIRED )

protobuf_generate_cpp(PROTO_SRC PROTO_HDR "KinectFileDef.proto")

## Recorder
set (SOURCES
	"KinectFileDef.proto"
	"Recorder/main.cpp"
	"Recorder/Kinect/Kinect.h"
	"Recorder/Kinect/Kinect.cpp"
	)
add_executable( Recorder "${SOURCES};${PROTO_SRC}" "${PROTO_HDR}" )
set_target_properties( Recorder PROPERTIES
	RESOURCE "KinectFileDef.proto"
	)
	
set_property( DIRECTORY PROPERTY 
	VS_STARTUP_PROJECT "Recorder"
	)

target_include_directories(Recorder 
	PUBLIC 
	${KinectSDK_INCLUDE_DIRS} 
	${PROTOBUF_INCLUDE_DIR}
	PRIVATE 
	${CMAKE_CURRENT_BINARY_DIR}
	)
target_link_libraries(Recorder PUBLIC
	${KinectSDK_LIBRARIES} 
	protobuf::libprotobuf
	)

## Player
set (SOURCES
	"KinectFileDef.proto"
	"Player/main.cpp"
	"Player/reader.h"
	"Player/reader.cpp"
	"Player/render.h"
	"Player/render.cpp"
	)
	
option (BUILD_PLAYER OFF)	
if(BUILD_PLAYER)
	add_executable( Player WIN32 "${SOURCES};${PROTO_SRC}" "${PROTO_HDR}")
	set_target_properties( Player PROPERTIES
		RESOURCE "KinectFileDef.proto"
		)
	
	target_include_directories(Player 
		PUBLIC 
		${KinectSDK_INCLUDE_DIRS} 
		${PROTOBUF_INCLUDE_DIR}
		PRIVATE 
		${CMAKE_CURRENT_BINARY_DIR}
		)
	target_link_libraries(Player 
		PUBLIC
		${KinectSDK_LIBRARIES} 
		protobuf::libprotobuf
		)
		
	target_compile_definitions(Player
		PUBLIC
		"-D_UNICODE"
		"_DNOMINMAX"
		)
endif(BUILD_PLAYER)