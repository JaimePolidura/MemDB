set(BINARY ${CMAKE_PROJECT_NAME})
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)
file(GLOB_RECURSE REMOVE_CMAKE "build/*")
list(REMOVE_ITEM SOURCES ${REMOVE_CMAKE})

add_executable(${BINARY}_run ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES} MemDb.cpp MemDbCreator.cpp utils/Utils.h
        db/MemDbStores.h
        db/MemDbStores.h
        db/AVLTree.h
)

# VCPKG Libraries
# He cambiado el nombre de la carpeta de boost a xd. Con eso el programa no se bloquea
include_directories(${CMAKE_PREFIX_PATH}/include)

# No funciona en windows y no se por que
if(UNIX)
    find_package(spdlog CONFIG REQUIRED)
    target_link_libraries(${BINARY}_lib PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
endif()

if(UNIX)
    target_include_directories(${BINARY}_lib PRIVATE /usr/local/include)
    target_link_directories(${BINARY}_lib PRIVATE /usr/local/lib)
    target_link_libraries(${BINARY}_lib PRIVATE -lcpprest -lcrypto -lz)
    target_link_libraries(${BINARY}_run PRIVATE ${BINARY}_lib)
endif()

target_precompile_headers(${BINARY}_lib PRIVATE shared.h)

if(NOT UNIX)
    target_link_libraries(${BINARY}_run PRIVATE ws2_32 wsock32)
else()
    target_link_libraries(${BINARY}_run PRIVATE pthread)
    add_compile_options(-pthread -lboost_thread -lboost_thread-mt -lz)
endif()

target_link_libraries(${BINARY}_run PRIVATE ${BINARY}_lib)
