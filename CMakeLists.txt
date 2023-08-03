cmake_minimum_required(VERSION 3.5)

project(ext_taskbar VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS WebEngineWidgets)

if (MSVC)
    add_compile_options("/Zc:preprocessor")
endif()

file(GLOB_RECURSE EXT_SOURCES
    C:/cpp/ext/ext_infrastructure/*.hpp
    C:/cpp/ext/ext_infrastructure/*.cpp
    C:/cpp/ext/ext_core/*.hpp
    C:/cpp/ext/ext_core/*.cpp
    C:/cpp/ext/ext_gui/*.hpp
    C:/cpp/ext/ext_gui/*.cpp
    C:/cpp/ext/ext_widgets/*.hpp
    C:/cpp/ext/ext_widgets/*.cpp
    C:/cpp/ext/ext_web/*.hpp
    C:/cpp/ext/ext_web/*.cpp
)

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

        ${EXT_SOURCES}
)

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

target_link_libraries(ext_taskbar PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(ext_taskbar PRIVATE Qt${QT_VERSION_MAJOR}::WebEngineWidgets)

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

target_include_directories(ext_taskbar PRIVATE "C:/cpp/ext")

target_include_directories(ext_taskbar PRIVATE "C:/cpp/windows/boost_1_82_0")
target_link_directories(ext_taskbar PRIVATE "C:/cpp/windows/boost_1_82_0/stage/lib")
#target_include_directories(ext_taskbar PRIVATE "C:/Program Files/Graphviz/include")
#target_link_directories(ext_taskbar PRIVATE "C:/Program Files/Graphviz/lib")

#target_compile_definitions(ext_taskbar PUBLIC _USE_JOIN_VIEW_INPUT_RANGE)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5105")
    string(REGEX REPLACE "/W[1-3]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()
