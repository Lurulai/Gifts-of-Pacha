cmake_minimum_required(VERSION 3.27)
project(GiftsofPacha)

set(CMAKE_CXX_STANDARD 20)

add_executable(GiftsofPacha main.cpp)

# Copy data.txt to the build directory
 configure_file(data.txt ${CMAKE_BINARY_DIR}/data.txt COPYONLY)
