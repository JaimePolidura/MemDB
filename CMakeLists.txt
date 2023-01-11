cmake_minimum_required(VERSION 3.23)
project(memdb)

set(CMAKE_CXX_STANDARD 23)

include_directories(src)

add_subdirectory(src)
add_subdirectory(tst)
add_subdirectory(playground)
add_subdirectory(libs/googletest)