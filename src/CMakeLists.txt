find_package(Boost 1.80.0)
include_directories(${Boost_INCLUDE_DIRS})

set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

set(SOURCES ${SOURCES})

add_executable(${BINARY}_run ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES})

if(MINGW)
    target_link_libraries(${BINARY}_run ws2_32)
    target_link_libraries(${BINARY}_run wsock32)
endif()