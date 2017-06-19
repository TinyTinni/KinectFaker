cmake_minimum_required( VERSION 3.1 )
project( KinectPlayer )

## (optional) Player - plays ksk files
set (SOURCES
	"main.cpp"
	"reader.h"
	"reader.cpp"
	"render.h"
	"render.cpp"
	)
add_executable( Player WIN32 "${SOURCES}")

target_include_directories(Player 
	PUBLIC 
	${KinectSDK_INCLUDE_DIRS}
	PRIVATE 
	${CMAKE_CURRENT_BINARY_DIR}
	)
target_link_libraries(Player 
PUBLIC
	${KinectSDK_LIBRARIES}
	protobuf_def
	)
	
target_compile_definitions(Player
	PUBLIC
	-D_UNICODE
	-DNOMINMAX
	)
	
# get the proxy dll
add_custom_command(TARGET Player POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:Kinect10>              
    $<TARGET_FILE_DIR:Player>)

add_custom_command(TARGET Player POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:Kinect10>              
    $<TARGET_FILE_DIR:Player>)
	
add_dependencies(Player Kinect10)