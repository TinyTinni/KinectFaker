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
	${KINECT_FAKE_INCLUDE_DIR}
	PRIVATE 
	${CMAKE_CURRENT_BINARY_DIR}
	)
target_link_libraries(Player 
PUBLIC
	${KINECT_FAKE_LIBRARIES}
	)
	
target_compile_definitions(Player
	PUBLIC
	-D_UNICODE
	-DNOMINMAX
	)