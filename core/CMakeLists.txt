cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 23)
project(memdb)

option(BUILDING_LINUX_DEPLOYMENT OFF)

if(NOT BUILDING_LINUX_DEPLOYMENT)
    set(VCPKG_ROOT "C:/vcpkg")
    set(VCPKG_TARGET_TRIPLET "x64-windows")
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows")
endif()

include_directories(src)
add_subdirectory(src)

if(NOT BUILDING_LINUX_DEPLOYMENT)
    add_subdirectory(tst)
    add_subdirectory(playground)
    add_subdirectory(libs/googletest)
endif()