set(BINARY ${CMAKE_PROJECT_NAME})
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)
add_executable(${BINARY}_run ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES} cluster/othernodes/NodeGroupOptions.h cluster/changehandler/partition/DeletionNodeChangeHandler.h)

# VCPKG Libraries
# He cambiado el nombre de la carpeta de boost a xd. Con eso el programa no se bloquea
include_directories(${CMAKE_PREFIX_PATH}/include)

find_package(Boost REQUIRED)
target_link_libraries(${BINARY}_lib PRIVATE Boost::boost)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${BINARY}_lib PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${BINARY}_lib PRIVATE nlohmann_json::nlohmann_json)

if(NOT BUILDING_LINUX_DEPLOYMENT)
    find_package(etcd-cpp-api CONFIG)
    target_link_libraries(${BINARY}_lib PRIVATE etcd-cpp-api)
else()
    target_include_directories(${BINARY}_lib PRIVATE /usr/local/include)
    target_link_libraries(${BINARY}_lib PRIVATE cpprest etcd-cpp-api)
    target_link_directories(${BINARY}_lib PRIVATE /usr/local/lib)
    target_compile_options(${BINARY}_lib PRIVATE -I/usr/local/include/etcd)
    target_link_libraries(${BINARY}_lib PRIVATE -letcd-cpp-api -lcpprest)
    target_link_libraries(${BINARY}_run PRIVATE ${BINARY}_lib)
endif()

target_precompile_headers(${BINARY}_lib PRIVATE shared.h)

if(NOT BUILDING_LINUX_DEPLOYMENT)
    target_link_libraries(${BINARY}_run PRIVATE ws2_32 wsock32)
else()
    target_link_libraries(${BINARY}_run PRIVATE pthread)
    add_compile_options(-pthread -lboost_thread -lboost_thread-mt)
endif()

target_link_libraries(${BINARY}_run PRIVATE ${BINARY}_lib)