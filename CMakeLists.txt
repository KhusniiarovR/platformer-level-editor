cmake_minimum_required(VERSION 3.22)
project(level-editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(level-editor main.cpp main_window.h main_window.cpp
                  utilities.h tile_icon_manager.h direction_input_widget.h)
target_link_libraries(level-editor PRIVATE Qt6::Widgets)
