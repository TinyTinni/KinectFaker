# Example CMakeLists.txt
# FindKInectSDK.cmake copy to CMake\share\cmake-3.5\Modules or same directory as this file

cmake_minimum_required( VERSION 3.1 )
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} )

project( KinectRecorder )

# Find Kinect SDK v1
find_package( KinectSDK REQUIRED )

find_package( Protobuf REQUIRED )

protobuf_generate_cpp(PROTO_SRC PROTO_HDR "KinectFileDef.proto")

## Faker library
set(SUBHOOK_NO_TESTS ON)
set(SUBHOOK_STATIC ON)

add_subdirectory("Faker/subhook")

add_library(protobuf_def ${PROTO_SRC} "${PROTO_HDR};KinectFileDef.proto")
target_include_directories(protobuf_def PUBLIC ${PROTOBUF_INCLUDE_DIR})
target_link_libraries(protobuf_def PUBLIC protobuf::libprotobuf)

set (SOURCES
	"Readme.md"
	
	"Faker/NuiApi_Faker.h"
	"Faker/NuiApi_Faker.cpp"
	"Faker/NuiSensor_Faker.h"
	"Faker/NuiSensor_Faker.cpp"
	)
	
add_library( Kinect_Faker "${SOURCES}")

target_include_directories(Kinect_Faker  
	PUBLIC
	${KinectSDK_INCLUDE_DIRS} 
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR} # for proto
	#submodules
	"Faker/subhook"
	)
target_link_libraries(Kinect_Faker
	protobuf_def
	subhook
	${KinectSDK_LIBRARIES}
	)
	
## Recorder
set (SOURCES
	"Recorder/main.cpp"
	"Recorder/Kinect/Kinect.h"
	"Recorder/Kinect/Kinect.cpp"
	)
add_executable( Recorder "${SOURCES}" )
set_target_properties( Recorder PROPERTIES
	RESOURCE "KinectFileDef.proto"
	)
	
set_property( DIRECTORY PROPERTY 
	VS_STARTUP_PROJECT "Recorder"
	)

target_include_directories(Recorder 
	PUBLIC 
	${KinectSDK_INCLUDE_DIRS}
	PRIVATE 
	${CMAKE_CURRENT_BINARY_DIR}
	)
target_link_libraries(Recorder PUBLIC
	${KinectSDK_LIBRARIES} 
	protobuf_def
	)

## (optional) Player - plays ksk files
set (SOURCES
	"Player/main.cpp"
	"Player/reader.h"
	"Player/reader.cpp"
	"Player/render.h"
	"Player/render.cpp"
	)
	
option (BUILD_PLAYER OFF)	
if(BUILD_PLAYER)
	add_executable( Player WIN32 "${SOURCES}")
	set_target_properties( Player PROPERTIES
		RESOURCE "KinectFileDef.proto"
		)
	
	target_include_directories(Player 
		PUBLIC 
		${KinectSDK_INCLUDE_DIRS}
		"Faker"
		PRIVATE 
		${CMAKE_CURRENT_BINARY_DIR}
		)
	target_link_libraries(Player 
		PUBLIC
		Kinect_Faker
		)
		
	target_compile_definitions(Player
		PUBLIC
		-D_UNICODE
		-DNOMINMAX
		)
endif(BUILD_PLAYER)