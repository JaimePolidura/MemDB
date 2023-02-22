find_package(Boost 1.80.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(BINARY ${CMAKE_PROJECT_NAME})
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)
set(SOURCES ${SOURCES})
add_executable(${BINARY}_run ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES})
target_link_libraries(${BINARY}_lib ${Boost_LIBRARIES})

if(NOT BUILDING_LINUX_DEPLOYMENT)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()
include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400)
FetchContent_MakeAvailable(cpr)
target_link_libraries(${BINARY}_run PRIVATE cpr::cpr)

# VCPKG Libraries
if(NOT BUILDING_LINUX_DEPLOYMENT)
    set(VCPKG_ROOT "C:/vcpkg")
    set(VCPKG_TARGET_TRIPLET "x64-windows")
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows")
endif()

include_directories(${CMAKE_PREFIX_PATH}/include)

find_package(etcd-cpp-api CONFIG REQUIRED)
target_link_libraries(${BINARY}_lib PRIVATE etcd-cpp-api)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${BINARY}_lib PRIVATE nlohmann_json::nlohmann_json)

if(NOT BUILDING_LINUX_DEPLOYMENT)
    target_link_libraries(${BINARY}_run PRIVATE ws2_32 wsock32)
else()
    target_link_libraries(${BINARY}_run PRIVATE pthread)
    add_compile_options(-pthread -lboost_thread -lboost_thread-mt)
endif()