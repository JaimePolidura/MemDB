# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/programacion/MemDB/cmake-build-debug/_deps/asio-cmake-src/asio-asio-1-12-2-src"
  "C:/programacion/MemDB/cmake-build-debug/_deps/asio-build"
  "C:/programacion/MemDB/cmake-build-debug/_deps/asio-subbuild/asio-populate-prefix"
  "C:/programacion/MemDB/cmake-build-debug/_deps/asio-subbuild/asio-populate-prefix/tmp"
  "C:/programacion/MemDB/cmake-build-debug/_deps/asio-subbuild/asio-populate-prefix/src/asio-populate-stamp"
  "C:/programacion/MemDB/cmake-build-debug/_deps/asio-subbuild/asio-populate-prefix/src"
  "C:/programacion/MemDB/cmake-build-debug/_deps/asio-subbuild/asio-populate-prefix/src/asio-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/programacion/MemDB/cmake-build-debug/_deps/asio-subbuild/asio-populate-prefix/src/asio-populate-stamp/${subDir}")
endforeach()