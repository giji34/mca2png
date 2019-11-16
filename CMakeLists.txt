cmake_minimum_required(VERSION 3.0)
project(mca2png)

include_directories(ext/libminecraft-file/include ext/svpng ext/hwm.task ext/colormap-shaders/include)
add_definitions(-std=c++17)
add_executable(mca2png src/main.cpp src/block_color.cpp src/block_color.h src/color.h ext/libminecraft-file/include/minecraft-file.hpp)
target_link_libraries(mca2png z)
