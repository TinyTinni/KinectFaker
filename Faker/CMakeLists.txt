cmake_minimum_required( VERSION 3.1 )
project( Kinect_Fake )

## Faker library
set(SUBHOOK_NO_TESTS ON PARENT_SCOPE)
set(SUBHOOK_STATIC ON PARENT_SCOPE)
add_subdirectory("subhook")

set (SOURCES
    "${CMAKE_SOURCE_DIR}/Readme.md"
    "NuiApi_Faker.h"
    "NuiApi_Faker.cpp"
    "NuiSensor_Faker.h"
    "NuiSensor_Faker.cpp"
    )

add_library( Kinect_Fake "${SOURCES}")
target_include_directories(Kinect_Fake  
PUBLIC
    ${KinectSDK_INCLUDE_DIRS} 
PRIVATE
    ${CMAKE_BINARY_DIR} # for proto
    #submodules
    "subhook"
    )
target_link_libraries(Kinect_Fake
PUBLIC
    protobuf_def    
    subhook
    ${KinectSDK_LIBRARIES}
    )
    
set(KINECT_FAKE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(KINECT_FAKE_LIBRARIES Kinect_Fake PARENT_SCOPE)