cmake_minimum_required(VERSION 3.14)

set(PROJ_NAME "app_qt")

project(${PROJ_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/conan)
include_directories(${CMAKE_BINARY_DIR}/conan/include)
link_directories(${CMAKE_BINARY_DIR}/conan/lib)


find_package(Qt5 REQUIRED COMPONENTS Core Gui Quick QuickControls2)

include_directories(
    # QT includes
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5QuickControls2_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}
    main.cpp
    resources.qrc
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Quick
    Qt5::QuickControls2
)
