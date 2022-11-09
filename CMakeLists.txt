cmake_minimum_required(VERSION 3.23)
project(memdb)

set(CMAKE_CXX_STANDARD 23)

add_executable(memdb main.cpp src/utils/DBMap.cpp src/utils/HashCreator.hpp src/utils/Map.h src/utils/PolynomialHashCreator.cpp)
