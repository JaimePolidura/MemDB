cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 23)
project(memdb)

option(BUILDING_LINUX_DEPLOYMENT OFF)

include_directories(src)
add_subdirectory(src)

if(NOT BUILDING_LINUX_DEPLOYMENT)
    add_subdirectory(tst)
    add_subdirectory(playground)
    add_subdirectory(libs/googletest)
endif()