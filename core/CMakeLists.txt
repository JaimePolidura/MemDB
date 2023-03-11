cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 23)
project(memdb)

option(BUILDING_LINUX_DEPLOYMENT OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

if(NOT BUILDING_LINUX_DEPLOYMENT)
    set(VCPKG_ROOT "C:/vcpkg")
    set(VCPKG_TARGET_TRIPLET "x64-windows")
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows")
endif()
if(BUILDING_LINUX_DEPLOYMENT)
    set(VCPKG_ROOT "opt/vcpkg")
    set(VCPKG_TARGET_TRIPLET "x64-linux")
    set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-linux")
endif()

include(cotire)

include_directories(src)
add_subdirectory(src)

if(NOT BUILDING_LINUX_DEPLOYMENT)
    add_subdirectory(tst)
    add_subdirectory(playground)
endif()