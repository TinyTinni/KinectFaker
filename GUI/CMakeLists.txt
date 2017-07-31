cmake_minimum_required( VERSION 3.1 )
project( KinectProxyGUI )


find_package( KinectSDK REQUIRED )

myhunter_add_package(Protobuf)
find_package( Protobuf REQUIRED )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

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
    "main.cpp"
    "main_window.h"
    "main_window.cpp"
    "../Proxy/NuiSensor_Faker.h"
    "../Proxy/NuiSensor_Faker.cpp"
    "SkeletonViewer.h"
    "SkeletonViewer.cpp"
    "Kinect.h"
    "Kinect.cpp"
    )
add_executable( GUI WIN32 ${SOURCES} ${UI_SOURCES})

target_include_directories(GUI 
    PUBLIC 
    ${KinectSDK_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    )
    
    
target_link_libraries(GUI 
    PUBLIC
    ${KinectSDK_LIBRARIES}
    protobuf_def
    Qt5::Widgets
    )

target_compile_definitions(GUI 
    PUBLIC
    NOMINMAX
    )
    

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

    install(
        PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    )
    
    install(TARGETS GUI
        RUNTIME DESTINATION ./GUI
    )
    
endif()