cmake_minimum_required(VERSION 4.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Nebula LANGUAGES C CXX)

set(NBL_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

option(NBL_BUILD_FRONTEND "Build Frontend" ON)
option(NBL_BUILD_TESTS "Build Tests" ON)
option(NBL_BUILD_TEST_PLUGIN "Build Test Plugin" ON)
option(NBL_BUILD_SGDB_PLUGIN "Build SteamGridDB Plugin" ON)
option(NBL_BUILD_IGDB_PLUGIN "Build IGDB.com Plugin" ON)

#
# Dependencies
#
include(cmake/CPM.cmake)
CPMAddPackage(
    NAME googletest
    GIT_REPOSITORY "https://github.com/google/googletest"
    GIT_TAG "v1.17.0"
    OPTIONS
        "INSTALL_GTEST OFF"
        "BUILD_GMOCK OFF"
)

CPMAddPackage(
    NAME curl
    GIT_REPOSITORY "https://github.com/curl/curl"
    GIT_TAG "curl-8_20_0"
    OPTIONS
        "BUILD_CURL_EXE OFF"
        "BUILD_SHARED_LIBS OFF"
        "CMAKE_USE_OPENSSL ON"
        "CMAKE_POSITION_INDEPENDENT_CODE ON"
)

include(cmake/SQLite3.cmake)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#
# Targets
#

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /Entry:WinMainCRTStartup")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /Subsystem:Windows")
endif()

set(NBL_SRC_DIR ${NBL_ROOT_DIR}/src)
set(NBL_INCLUDE_DIR ${NBL_ROOT_DIR}/include)

file(GLOB_RECURSE NBL_API_SOURCES ${NBL_SRC_DIR}/api/**.cpp)
file(GLOB_RECURSE NBL_API_HEADERS ${NBL_INCLUDE_DIR}/api/**.hpp)

add_library(Nebula_Api SHARED ${NBL_API_SOURCES})
target_include_directories(Nebula_Api PUBLIC ${NBL_INCLUDE_DIR})
target_include_directories(Nebula_Api PRIVATE ${NBL_SRC_DIR}/api)
target_precompile_headers(Nebula_Api PUBLIC ${NBL_API_HEADERS})
target_link_libraries(Nebula_Api PRIVATE CURL::libcurl)
target_link_libraries(Nebula_Api PRIVATE SQLite3::SQLite3)

file(GLOB_RECURSE NBL_WRAPPER_SOURCES ${NBL_SRC_DIR}/wrapper/**.cpp)

add_executable(Nebula_Wrapper ${NBL_WRAPPER_SOURCES})
target_include_directories(Nebula_Wrapper PRIVATE ${NBL_SRC_DIR}/wrapper)
target_link_libraries(Nebula_Wrapper PRIVATE Nebula_Api)

if(${NBL_BUILD_TESTS}) 
    file(GLOB_RECURSE NBL_TEST_SOURCES ${NBL_SRC_DIR}/test/**.cpp)
    add_executable(Nebula_Test ${NBL_TEST_SOURCES})
    target_link_libraries(Nebula_Test PRIVATE Nebula_Api)
    target_link_libraries(Nebula_Test PRIVATE GTest::gtest_main)
endif()

if(${NBL_BUILD_TEST_PLUGIN}) 

    file(GLOB_RECURSE NBL_TEST_PLUGIN_SOURCES ${NBL_SRC_DIR}/test_plugin/**.cpp)
    add_library(Nebula_TestPlugin SHARED ${NBL_TEST_PLUGIN_SOURCES})
    target_link_libraries(Nebula_TestPlugin PRIVATE Nebula_Api)
    set_target_properties(Nebula_TestPlugin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/test_plugin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/test_plugin"
        PREFIX ""
        OUTPUT_NAME "plugin"
    )

    add_custom_command(
        TARGET Nebula_TestPlugin
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${NBL_SRC_DIR}/test_plugin/plugin.json" "${CMAKE_BINARY_DIR}/bin/plugins/test_plugin"
        VERBATIM
    )

endif()

if(${NBL_BUILD_SGDB_PLUGIN}) 

    file(GLOB_RECURSE NBL_SGDB_PLUGIN_SOURCES ${NBL_SRC_DIR}/plugin_sgdb/**.cpp)
    add_library(Nebula_SgdbPlugin SHARED ${NBL_SGDB_PLUGIN_SOURCES})
    target_link_libraries(Nebula_SgdbPlugin PRIVATE Nebula_Api)
    set_target_properties(Nebula_SgdbPlugin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/sgdb"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/sgdb"
        PREFIX ""
        OUTPUT_NAME "plugin"
    )

    add_custom_command(
        TARGET Nebula_SgdbPlugin
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${NBL_SRC_DIR}/plugin_sgdb/plugin.json" "${CMAKE_BINARY_DIR}/bin/plugins/sgdb"
        VERBATIM
    )

endif()

if(${NBL_BUILD_IGDB_PLUGIN}) 

    file(GLOB_RECURSE NBL_IGDB_PLUGIN_SOURCES ${NBL_SRC_DIR}/plugin_igdb/**.cpp)
    add_library(Nebula_IgdbPlugin SHARED ${NBL_IGDB_PLUGIN_SOURCES})
    target_link_libraries(Nebula_IgdbPlugin PRIVATE Nebula_Api)
    set_target_properties(Nebula_IgdbPlugin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/igdb"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/igdb"
        PREFIX ""
        OUTPUT_NAME "plugin"
    )

    add_custom_command(
        TARGET Nebula_IgdbPlugin
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${NBL_SRC_DIR}/plugin_igdb/plugin.json" "${CMAKE_BINARY_DIR}/bin/plugins/igdb"
        VERBATIM
    )

endif()

if(${NBL_BUILD_FRONTEND}) 
    add_subdirectory(src/frontend)
endif()

