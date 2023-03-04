find_package(Boost 1.80.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(BINARY ${CMAKE_PROJECT_NAME})
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)
add_executable(${BINARY}_run ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES})
target_link_libraries(${BINARY}_lib ${Boost_LIBRARIES})

# VCPKG Libraries
include_directories(${CMAKE_PREFIX_PATH}/include)

find_package(etcd-cpp-api CONFIG REQUIRED)
target_link_libraries(${BINARY}_lib PRIVATE etcd-cpp-api)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${BINARY}_lib PRIVATE nlohmann_json::nlohmann_json)

target_link_libraries(${BINARY}_run PRIVATE ${BINARY}_lib)

if(NOT BUILDING_LINUX_DEPLOYMENT)
    target_link_libraries(${BINARY}_run PRIVATE ws2_32 wsock32)
else()
    target_link_libraries(${BINARY}_run PRIVATE pthread)
    add_compile_options(-pthread -lboost_thread -lboost_thread-mt)
endif()