find_package(Boost 1.80.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(BINARY ${CMAKE_PROJECT_NAME})
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)
set(SOURCES ${SOURCES})
add_executable(${BINARY}_run ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES})
target_link_libraries(${BINARY}_lib ${Boost_LIBRARIES})

include_directories(${CMAKE_PREFIX_PATH}/include)

find_package(etcd-cpp-api CONFIG REQUIRED)
target_link_libraries(${BINARY}_lib PRIVATE etcd-cpp-api)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${BINARY}_lib PRIVATE nlohmann_json::nlohmann_json)

set(CPR_INCLUDE_DIR C:\\libs\\C++\\cpr\\include)
find_package(cpr REQUIRED)
target_link_libraries(${BINARY}_lib PRIVATE cpr)
target_include_directories(${BINARY}_lib PRIVATE ${CPR_INCLUDE_DIR})

add_compile_options(-lcpr -lcurl)

if(NOT BUILDING_LINUX_DEPLOYMENT)
    target_link_libraries(${BINARY}_run ws2_32)
    target_link_libraries(${BINARY}_run wsock32)
else()
    target_link_libraries(${BINARY}_run pthread)
    add_compile_options(-pthread -lboost_thread -lboost_thread-mt)
endif()