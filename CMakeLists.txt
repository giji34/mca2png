cmake_minimum_required(VERSION 3.0)
project(mca2png)

include(CheckCCompilerFlag)

if (NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

include_directories(ext/libminecraft-file/include
                    ext/zopfli/src/zopflipng/lodepng
                    ext/colormap-shaders/include
                    ext/zopfli/src/zopflipng
                    ext/hwm.task)
add_executable(mca2png src/main.cpp
                       src/block_color.cpp
                       src/block_color.h
                       src/color.h
                       ext/libminecraft-file/include/minecraft-file.hpp
                       ext/zopfli/src/zopflipng/lodepng/lodepng.h
                       ext/zopfli/src/zopflipng/lodepng/lodepng.cpp
                       ext/zopfli/src/zopflipng/lodepng/lodepng_util.h
                       ext/zopfli/src/zopflipng/lodepng/lodepng_util.cpp
                       ext/zopfli/src/zopflipng/zopflipng_lib.cc
                       ext/zopfli/src/zopflipng/zopflipng_lib.h
                       ext/zopfli/src/zopfli/blocksplitter.c
                       ext/zopfli/src/zopfli/cache.c
                       ext/zopfli/src/zopfli/deflate.c
                       ext/zopfli/src/zopfli/gzip_container.c
                       ext/zopfli/src/zopfli/hash.c
                       ext/zopfli/src/zopfli/katajainen.c
                       ext/zopfli/src/zopfli/lz77.c
                       ext/zopfli/src/zopfli/squeeze.c
                       ext/zopfli/src/zopfli/tree.c
                       ext/zopfli/src/zopfli/util.c
                       ext/zopfli/src/zopfli/zlib_container.c
                       ext/zopfli/src/zopfli/zopfli_lib.c)
list(APPEND mca2png_link_libraries "z")

set(CMAKE_REQUIRED_FLAGS "-lstdc++fs")
check_c_compiler_flag("" TEST_RESULT_STDCXXFS)
if (TEST_RESULT_STDCXXFS AND "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  list(APPEND mca2png_link_libraries stdc++fs)
endif()

set(CMAKE_REQUIRED_FLAGS "-pthread")
check_c_compiler_flag("" TEST_RESULT_PTHREAD)
if (TEST_RESULT_PTHREAD)
  list(APPEND mca2png_link_libraries pthread)
endif()

target_link_libraries(mca2png ${mca2png_link_libraries})
