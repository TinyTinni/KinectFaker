cmake_minimum_required( VERSION 3.1 )
project( KinectProxyGUI )

###############################################################
# packages
###############################################################
find_package( KinectSDK REQUIRED )

myhunter_add_package(Protobuf)
find_package( Protobuf REQUIRED )

find_package(FFMPEG REQUIRED COMPONENTS SWSCALE AVUTIL AVFORMAT AVCODEC)

if (NOT OUTCOME_INCLUDE_DIR)
    set(OUTCOME_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/outcome" CACHE PATH "Path to outcome header")
    message(STATUS "Outcome include dir was not specified. Download it.")
    file(DOWNLOAD "https://raw.githubusercontent.com/ned14/outcome/master/single-header/outcome.hpp" "${OUTCOME_INCLUDE_DIR}/outcome.hpp")
endif()
###############################################################

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

if(POLICY CMP0071)
    cmake_policy(GET CMP0071 _ALREDAY_SET)
    if (NOT _ALREADY_SET)
        cmake_policy(SET CMP0071 NEW)
    endif()
endif()

# Find the QtWidgets library
#myhunter_add_package(Qt5 COMPONENTS Widgets)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

set (UIS "main_window.ui"
)

QT5_WRAP_UI(UI_SOURCES 
    ${UIS}
    )


## (optional) GUI
set (SOURCES
    "Kinect.h"
    "Kinect.cpp"
    "main.cpp"
    "main_window.h"
    "main_window.cpp"
    "SkeletonViewer.h"
    "SkeletonViewer.cpp"
    )
add_executable( GUI WIN32 ${SOURCES} ${UI_SOURCES})

target_include_directories(GUI 
    PUBLIC 
    ${CMAKE_BINARY_DIR}
    
    ${KinectSDK_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
    ${OUTCOME_INCLUDE_DIR}
    )
    
target_link_libraries(GUI 
    PUBLIC
    ${KinectSDK_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    protobuf_def
    Qt5::Widgets
    )

target_compile_definitions(GUI 
    PUBLIC
    NOMINMAX
    )
    
###############################################################
# install
###############################################################
if(Qt5_FOUND AND WIN32 AND TARGET Qt5::qmake AND NOT TARGET Qt5::windeployqt)
    get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

    execute_process(
        COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt5_install_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")

    if(EXISTS ${imported_location})
        add_executable(Qt5::windeployqt IMPORTED)

        set_target_properties(Qt5::windeployqt PROPERTIES
            IMPORTED_LOCATION ${imported_location}
        )
    endif()
endif()   
    
if(TARGET Qt5::windeployqt)
    # execute windeployqt in a tmp directory after build
    add_custom_command(TARGET GUI
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
        COMMAND set PATH=%PATH%$<SEMICOLON>${qt5_install_prefix}/bin
        COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt" "$<TARGET_FILE_DIR:GUI>/$<TARGET_FILE_NAME:GUI>"
    )

    # copy deployment directory during installation
    install(
        DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
        DESTINATION ./GUI
    )
    
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)

    include(InstallRequiredSystemLibraries)
    
    install(TARGETS GUI
        RUNTIME DESTINATION ./GUI
    )
    
endif()