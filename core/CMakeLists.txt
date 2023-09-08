cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 23)
project(memdb CXX)

option(BUILDING_TEST OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

if(NOT UNIX)
    set(VCPKG_ROOT "C:/vcpkg")
    set(VCPKG_TARGET_TRIPLET "x64-windows")
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows")
endif()
if(UNIX)
    set(VCPKG_ROOT "/vcpkg")
    set(VCPKG_TARGET_TRIPLET "x64-linux")
    set(CMAKE_TOOLCHAIN_FILE "/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-linux")
endif()

include_directories(src)
add_subdirectory(src)

if(BUILDING_TEST)
    add_subdirectory(tst)
endif()
