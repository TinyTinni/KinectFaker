cmake_minimum_required( VERSION 3.1 )
project( KinectPlayer )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)

set (UIS "main_window.ui"
)

QT5_WRAP_UI(UI_SOURCES 
    ${UIS}
    )


## (optional) Player - plays ksk files
set (SOURCES
    "main.cpp"
    "main_window.h"
    "main_window.cpp"
    "reader.h"
    "reader.cpp"
    "render.h"
    "render.cpp"
    "../Proxy/NuiSensor_Faker.h"
    "../Proxy/NuiSensor_Faker.cpp"
    "SkeletonViewer.h"
    "SkeletonViewer.cpp"
    "Kinect.h"
    "Kinect.cpp"
    )
add_executable( Player WIN32 ${SOURCES} ${UI_SOURCES})

target_include_directories(Player 
    PUBLIC 
    ${KinectSDK_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    )
    
    
target_link_libraries(Player 
PUBLIC
    ${KinectSDK_LIBRARIES}
    protobuf_def
    Qt5::Widgets
    )

target_compile_definitions(Player PUBLIC
    -DNOMINMAX
    )