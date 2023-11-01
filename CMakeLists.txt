# Preamble
cmake_minimum_required(VERSION 3.28)
list(APPEND CMAKE_MESSAGE_CONTEXT "ext_taskbar")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF) # https://discourse.cmake.org/t/cmake-3-28-cmake-cxx-compiler-clang-scan-deps-notfound-not-found/9244
project(ext_taskbar
    VERSION 0.0.0
    DESCRIPTION "Restore Windows 11 taskbar's lost functionalities when switching it to multi-row & never-combining mode"
    LANGUAGES CXX
)


# Project wide setup
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Dependencies
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS WebEngineWidgets)

set(FETCHCONTENT_SOURCE_DIR_BOOST "C:/cpp/boost_1_83_0/boost")
set(FETCHCONTENT_SOURCE_DIR_EXT "C:/Users/jhcar/Documents/GitHub/ext")

include(FetchContent)
FetchContent_Declare(Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG 564e2ac16907019696cdaba8a93e3588ec596062 # boost-1.83.0
    GIT_SHALLOW TRUE
    EXCLUDE_FROM_ALL
    #SYSTEM
    FIND_PACKAGE_ARGS 1.83.0
)
FetchContent_Declare(ext
    GIT_REPOSITORY https://github.com/jhcarl0814/ext.git
    GIT_TAG main
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(Boost ext)


# Targets
add_custom_target(ext_taskbar_administrative_files)
target_sources(ext_taskbar_administrative_files PRIVATE "./README.md" "./readme_rsc/ext_taskbar20230803.gif" "./readme_rsc/ext_taskbar20230803.mp4" "./LICENSE")
set_source_files_properties("./README.md" "./readme_rsc/ext_taskbar20230803.gif" "./readme_rsc/ext_taskbar20230803.mp4" "./LICENSE" PROPERTIES HEADER_FILE_ONLY YES)
add_custom_target(ext_taskbar_tooling_support)
target_sources(ext_taskbar_tooling_support PRIVATE "./.clang-format")
set_source_files_properties("./.clang-format" PROPERTIES HEADER_FILE_ONLY YES)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.hpp
        quick_launch.cpp
        quick_launch.hpp
        start_menu.cpp
        start_menu.hpp
        mime_viewer.cpp
        mime_viewer.hpp
        icon_viewer.cpp
        icon_viewer.hpp
        task_manager.cpp
        task_manager.hpp

        unifont.qrc
        "./fonts/unifont-15.0.01.ttf"
)
set_source_files_properties("./fonts/unifont-15.0.01.ttf" PROPERTIES HEADER_FILE_ONLY YES)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ext_taskbar
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ext_taskbar APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ext_taskbar SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ext_taskbar
            ${PROJECT_SOURCES}
        )
    endif()
endif()

message("CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message("CMAKE_CXX_SIMULATE_ID: ${CMAKE_CXX_SIMULATE_ID}")
message("CMAKE_CXX_COMPILER_FRONTEND_VARIANT: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")
if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC") # https://discourse.cmake.org/t/how-to-determine-if-clang-or-clang-cl-is-used-on-windows/4129
    target_compile_options(ext_taskbar PRIVATE "/W4" "/WX")
else()
    target_compile_options(ext_taskbar PRIVATE "-Wall" "-Wextra" "-pedantic" "-Werror" "-Wno-language-extension-token")
endif()

target_link_libraries(ext_taskbar PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(ext_taskbar PRIVATE Qt${QT_VERSION_MAJOR}::WebEngineWidgets)
target_link_libraries(ext_taskbar
    PRIVATE
        ext::web
)

set_target_properties(ext_taskbar PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ext_taskbar)
endif()
