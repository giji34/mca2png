cmake_minimum_required(VERSION 3.0)
project(mca2png)

include(CheckCCompilerFlag)

include_directories(ext/libminecraft-file/include
                    ext/yspng-20140625/src
                    ext/colormap-shaders/include
                    ext/zopfli/src/zopflipng)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
add_executable(mca2png src/main.cpp
                       src/block_color.cpp
                       src/block_color.h
                       src/color.h
                       ext/yspng-20140625/src/yspng.cpp
                       ext/yspng-20140625/src/yspng.h
                       ext/yspng-20140625/src/yspngenc.cpp
                       ext/yspng-20140625/src/yspngenc.h
                       ext/libminecraft-file/include/minecraft-file.hpp
                       ext/zopfli/src/zopflipng/lodepng/lodepng.cpp
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
