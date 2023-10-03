cmake_minimum_required(VERSION 3.26)
enable_testing()
set(BINARY ${CMAKE_PROJECT_NAME}_tst)

include_directories(${CMAKE_PREFIX_PATH}/include)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.cpp *.h)
add_executable(${BINARY} ${TEST_SOURCES})

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${BINARY} PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main ${CMAKE_PROJECT_NAME}_lib)
add_test(NAME ${BINARY} COMMAND ${BINARY})