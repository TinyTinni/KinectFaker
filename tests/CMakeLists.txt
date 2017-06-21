cmake_minimum_required( VERSION 3.1 )
project( Tests )

#search catch
find_path(
    CATCH_INCLUDE_DIR 
    NAMES catch.hpp
    DOC "Catch include dir"
)

if (NOT CATCH_INCLUDE_DIR)
    message(STATUS "Could not find Catch. Create download target.")
    include(ExternalProject)
    find_package(Git REQUIRED)
    ExternalProject_Add(
        catch
        PREFIX ${CMAKE_BINARY_DIR}/catch
        GIT_REPOSITORY https://github.com/philsquared/Catch.git
        TIMEOUT 10
        UPDATE_COMMAND ${GIT_EXECUTABLE} pull
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
    )
    
    # Expose required variable (CATCH_INCLUDE_DIR) to parent scope
    ExternalProject_Get_Property(catch source_dir)
    set(CATCH_INCLUDE_DIR ${source_dir}/single_include)
endif()



set (SOURCES
    "main.cpp"
    "SkeletonPosition.cpp"
    )

enable_testing()	
add_executable( tests "${SOURCES}" )
add_test(NAME skeleton_position
    COMMAND tests -b
    )
target_include_directories(tests 
PUBLIC 
    ${KinectSDK_INCLUDE_DIRS}
    ${CATCH_INCLUDE_DIR}
)

target_link_libraries(tests PUBLIC
    ${KinectSDK_LIBRARIES}
    )
    
set_target_properties(tests PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY
    "$(TargetDir)"
    )
    
add_custom_command(TARGET tests PRE_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:Kinect10>              
    $<TARGET_FILE_DIR:tests>)
    
add_dependencies(tests Kinect10)
