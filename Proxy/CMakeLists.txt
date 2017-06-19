cmake_minimum_required( VERSION 3.1 )
project( Kinect_Proxy )

## Faker library

set (SOURCES
    "${CMAKE_SOURCE_DIR}/Readme.md"
    "NuiSensor_Faker.h"
    "NuiSensor_Faker.cpp"
	"Proxy.cpp"
	
	"Kinect10.def"
    )

add_library( Kinect10 SHARED "${SOURCES}")

target_include_directories(Kinect10  
PRIVATE
	${KinectSDK_INCLUDE_DIRS} 
    ${CMAKE_BINARY_DIR} # for proto
    #submodules
    )
target_link_libraries(Kinect10
PUBLIC
    protobuf_def    
    )