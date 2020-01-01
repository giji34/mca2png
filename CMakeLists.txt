cmake_minimum_required(VERSION 3.0)
project(mca2png)

include(CheckCCompilerFlag)

include_directories(ext/libminecraft-file/include
                    ext/svpng
                    ext/hwm.task
                    ext/colormap-shaders/include)
add_definitions(-std=c++17)
add_executable(mca2png src/main.cpp
                       src/block_color.cpp
                       src/block_color.h
                       src/color.h
                       ext/libminecraft-file/include/minecraft-file.hpp)
list(APPEND mca2png_link_libraries "z")

set(CMAKE_REQUIRED_FLAGS "-lstdc++fs")
check_c_compiler_flag("" TEST_RESULT_STDCXXFS)
if (TEST_RESULT_STDCXXFS)
  list(APPEND mca2png_link_libraries stdc++fs)
endif()

set(CMAKE_REQUIRED_FLAGS "-pthread")
check_c_compiler_flag("" TEST_RESULT_PTHREAD)
if (TEST_RESULT_PTHREAD)
  list(APPEND mca2png_link_libraries pthread)
endif()

target_link_libraries(mca2png ${mca2png_link_libraries})

