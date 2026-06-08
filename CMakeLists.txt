cmake_minimum_required(VERSION 4.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Nebula LANGUAGES C CXX)

set(NBL_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})


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
target_include_directories(Nebula_Api PUBLIC ${NBL_INCLUDE_DIR}/api)
target_include_directories(Nebula_Api PRIVATE ${NBL_SRC_DIR}/api)
target_precompile_headers(Nebula_Api PUBLIC ${NBL_API_HEADERS})
target_link_libraries(Nebula_Api PRIVATE CURL::libcurl)


file(GLOB_RECURSE NBL_MODEL_SOURCES ${NBL_SRC_DIR}/model/**.cpp)

add_library(Nebula_Model STATIC ${NBL_MODEL_SOURCES})
target_include_directories(Nebula_Model PUBLIC ${NBL_INCLUDE_DIR}/api)
target_include_directories(Nebula_Model PUBLIC ${NBL_INCLUDE_DIR}/model)
target_include_directories(Nebula_Model PRIVATE ${NBL_SRC_DIR}/model)
target_link_libraries(Nebula_Model PRIVATE SQLite3::SQLite3)
target_link_libraries(Nebula_Model PUBLIC Nebula_Api)


file(GLOB_RECURSE NBL_TEST_SOURCES ${NBL_SRC_DIR}/test/**.cpp)

add_executable(Nebula_Test ${NBL_TEST_SOURCES})
target_link_libraries(Nebula_Test PRIVATE Nebula_Model)
target_link_libraries(Nebula_Test PRIVATE GTest::gtest_main)

add_subdirectory(frontend)

