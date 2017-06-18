cmake_minimum_required( VERSION 3.1 )
project( KinectRecorder )

set (SOURCES
	"main.cpp"
	"Kinect/Kinect.h"
	"Kinect/Kinect.cpp"
	)
add_executable( Recorder "${SOURCES}" )
target_include_directories(Recorder 
PUBLIC 
	${KinectSDK_INCLUDE_DIRS}
PRIVATE 
	${CMAKE_BINARY_DIR}
	)

target_link_libraries(Recorder PUBLIC
	${KinectSDK_LIBRARIES} 
	protobuf_def
	)
