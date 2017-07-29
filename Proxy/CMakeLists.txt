cmake_minimum_required( VERSION 3.1 )
project( Kinect_Proxy )


set (SOURCES
    "${CMAKE_SOURCE_DIR}/Readme.md"
    "NuiSensor_Faker.h"
    "NuiSensor_Faker.cpp"
    "Proxy.cpp"
    
    "Kinect10.def"
    )

add_library( Kinect10 SHARED "${SOURCES}")
###############################################################
# kinect Headers
###############################################################
find_package( KinectSDK QUIET )
if (NOT KinectSDK_FOUND)
    find_path(KinectSDK_INCLUDE_DIRS
        NAMES "NuiApi.h"
        DOC "Kinect10 Header directory"
    )
endif()
###############################################################
# json
###############################################################

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
    add_dependencies(Kinect10 json)
endif()


###############################################################
# spdlog
###############################################################

find_path(
    SPDLOG_INCLUDE_DIR 
    NAMES spdlog.h
    PATH_SUFFIXES "spdlog"
    DOC "spdlog include dir"
)
if (NOT SPDLOG_INCLUDE_DIR)
    message(STATUS "Could not find spdlog. Create Download target.")
    include(ExternalProject)
    find_package(Git REQUIRED)
    ExternalProject_Add(
        spdlog
        PREFIX ${CMAKE_BINARY_DIR}/spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        TIMEOUT 10
        UPDATE_COMMAND ${GIT_EXECUTABLE} pull
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
    )
    
    # Expose required variable (CATCH_INCLUDE_DIR) to parent scope
    ExternalProject_Get_Property(spdlog source_dir)
    set(SPDLOG_INCLUDE_DIR ${source_dir}/include)
    add_dependencies(Kinect10 spdlog)
endif()


###############################################################

target_include_directories(Kinect10  
PRIVATE
    ${KinectSDK_INCLUDE_DIRS} 
    ${CMAKE_BINARY_DIR} # for proto
    ${JSON_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR}
    )
target_link_libraries(Kinect10
PUBLIC
    protobuf_def   
    debug comsuppwd.lib optimized comsuppw.lib #comsupport for _bstr_t
    )
    
target_compile_definitions(Kinect10
    PUBLIC
    -DNOMINMAX
    )