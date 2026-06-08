cmake_minimum_required(VERSION 4.0)

set(QT_QML_GENERATE_QMLLS_INI ON)

project(Nebula_App LANGUAGES CXX)

find_package(Qt6 6.11.1 COMPONENTS Quick REQUIRED)
qt_standard_project_setup(REQUIRES 6.11.1)

file(GLOB_RECURSE NBL_APP_QML_SCRIPTS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/**.qml")
file(GLOB_RECURSE NBL_APP_FONTS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/fonts/**.ttf")

file(GLOB_RECURSE NBL_APP_SOURCES RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/**.cpp" "${CMAKE_CURRENT_LIST_DIR}/**.hpp")
list(REMOVE_ITEM NBL_APP_SOURCES "${CMAKE_CURRENT_LIST_DIR}/Main.cpp")

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_executable(Nebula_App Main.cpp)
qt_add_qml_module(Nebula_App
    URI Nebula
    VERSION 1.0
    QML_FILES ${NBL_APP_QML_SCRIPTS}
    RESOURCES ${NBL_APP_FONTS}
    SOURCES ${NBL_APP_SOURCES}
)

target_link_libraries(Nebula_App PRIVATE Nebula_Api)
target_link_libraries(Nebula_App PRIVATE Nebula_Model)
target_link_libraries(Nebula_App PRIVATE Qt6::Quick)
target_link_libraries(Nebula_App PRIVATE CURL::libcurl)
