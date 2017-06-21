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

#search json
find_path(
    JSON_INCLUDE_DIR 
    NAMES json.hpp
    PATH_SUFFIXES "nlohmann"
    DOC "nlohmann-json include dir"
)

if (NOT JSON_INCLUDE_DIR)
    message(STATUS "Could not find nlohmann-json. Create Download target.")
    include(ExternalProject)
    find_package(Git REQUIRED)
    ExternalProject_Add(
        json
        PREFIX ${CMAKE_BINARY_DIR}/json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        TIMEOUT 10
        UPDATE_COMMAND ${GIT_EXECUTABLE} pull
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
    )
    
    # Expose required variable (CATCH_INCLUDE_DIR) to parent scope
    ExternalProject_Get_Property(json source_dir)
    set(JSON_INCLUDE_DIR ${source_dir}/src)
endif()

target_include_directories(Kinect10  
PRIVATE
    ${KinectSDK_INCLUDE_DIRS} 
    ${CMAKE_BINARY_DIR} # for proto
    ${JSON_INCLUDE_DIR}
    )
target_link_libraries(Kinect10
PUBLIC
    protobuf_def    
    )
target_compile_definitions(Kinect10
    PUBLIC
    -DNOMINMAX
    )