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

file(GLOB_RECURSE NBL_CORE_SOURCES ${NBL_SRC_DIR}/core/**.cpp)
file(GLOB_RECURSE NBL_TEST_SOURCES ${NBL_SRC_DIR}/test/**.cpp)

file(GLOB_RECURSE NBL_CORE_HEADERS ${NBL_INCLUDE_DIR}/core/**.h)

add_library(Nebula_Core STATIC ${NBL_CORE_SOURCES})
target_include_directories(Nebula_Core PUBLIC ${NBL_INCLUDE_DIR}/core)
target_include_directories(Nebula_Core PRIVATE ${NBL_SRC_DIR}/core)
target_link_libraries(Nebula_Core PRIVATE SQLite3::SQLite3)
target_precompile_headers(Nebula_Core PUBLIC ${NBL_CORE_HEADERS})

add_executable(Nebula_Test ${NBL_TEST_SOURCES})
target_link_libraries(Nebula_Test PRIVATE Nebula_Core)
target_link_libraries(Nebula_Test PRIVATE GTest::gtest_main)

add_subdirectory(frontend)

