# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.23

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\cmake\win\bin\cmake.exe

# The command to remove a file.
RM = C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\cmake\win\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\programacion\MemDB

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\programacion\MemDB\cmake-build-debug

# Include any dependencies generated for this target.
include src/CMakeFiles/memdb_lib.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/memdb_lib.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/memdb_lib.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/memdb_lib.dir/flags.make

src/CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.obj: src/CMakeFiles/memdb_lib.dir/flags.make
src/CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.obj: src/CMakeFiles/memdb_lib.dir/includes_CXX.rsp
src/CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.obj: ../src/utils/DBMap.cpp
src/CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.obj: src/CMakeFiles/memdb_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\programacion\MemDB\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.obj"
	cd /d C:\programacion\MemDB\cmake-build-debug\src && C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.obj -MF CMakeFiles\memdb_lib.dir\utils\DBMap.cpp.obj.d -o CMakeFiles\memdb_lib.dir\utils\DBMap.cpp.obj -c C:\programacion\MemDB\src\utils\DBMap.cpp

src/CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.i"
	cd /d C:\programacion\MemDB\cmake-build-debug\src && C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\programacion\MemDB\src\utils\DBMap.cpp > CMakeFiles\memdb_lib.dir\utils\DBMap.cpp.i

src/CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.s"
	cd /d C:\programacion\MemDB\cmake-build-debug\src && C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\programacion\MemDB\src\utils\DBMap.cpp -o CMakeFiles\memdb_lib.dir\utils\DBMap.cpp.s

src/CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.obj: src/CMakeFiles/memdb_lib.dir/flags.make
src/CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.obj: src/CMakeFiles/memdb_lib.dir/includes_CXX.rsp
src/CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.obj: ../src/utils/PolynomialHashCreator.cpp
src/CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.obj: src/CMakeFiles/memdb_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\programacion\MemDB\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.obj"
	cd /d C:\programacion\MemDB\cmake-build-debug\src && C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.obj -MF CMakeFiles\memdb_lib.dir\utils\PolynomialHashCreator.cpp.obj.d -o CMakeFiles\memdb_lib.dir\utils\PolynomialHashCreator.cpp.obj -c C:\programacion\MemDB\src\utils\PolynomialHashCreator.cpp

src/CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.i"
	cd /d C:\programacion\MemDB\cmake-build-debug\src && C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\programacion\MemDB\src\utils\PolynomialHashCreator.cpp > CMakeFiles\memdb_lib.dir\utils\PolynomialHashCreator.cpp.i

src/CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.s"
	cd /d C:\programacion\MemDB\cmake-build-debug\src && C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\programacion\MemDB\src\utils\PolynomialHashCreator.cpp -o CMakeFiles\memdb_lib.dir\utils\PolynomialHashCreator.cpp.s

# Object files for target memdb_lib
memdb_lib_OBJECTS = \
"CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.obj" \
"CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.obj"

# External object files for target memdb_lib
memdb_lib_EXTERNAL_OBJECTS =

src/libmemdb_lib.a: src/CMakeFiles/memdb_lib.dir/utils/DBMap.cpp.obj
src/libmemdb_lib.a: src/CMakeFiles/memdb_lib.dir/utils/PolynomialHashCreator.cpp.obj
src/libmemdb_lib.a: src/CMakeFiles/memdb_lib.dir/build.make
src/libmemdb_lib.a: src/CMakeFiles/memdb_lib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\programacion\MemDB\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libmemdb_lib.a"
	cd /d C:\programacion\MemDB\cmake-build-debug\src && $(CMAKE_COMMAND) -P CMakeFiles\memdb_lib.dir\cmake_clean_target.cmake
	cd /d C:\programacion\MemDB\cmake-build-debug\src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\memdb_lib.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/memdb_lib.dir/build: src/libmemdb_lib.a
.PHONY : src/CMakeFiles/memdb_lib.dir/build

src/CMakeFiles/memdb_lib.dir/clean:
	cd /d C:\programacion\MemDB\cmake-build-debug\src && $(CMAKE_COMMAND) -P CMakeFiles\memdb_lib.dir\cmake_clean.cmake
.PHONY : src/CMakeFiles/memdb_lib.dir/clean

src/CMakeFiles/memdb_lib.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\programacion\MemDB C:\programacion\MemDB\src C:\programacion\MemDB\cmake-build-debug C:\programacion\MemDB\cmake-build-debug\src C:\programacion\MemDB\cmake-build-debug\src\CMakeFiles\memdb_lib.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/memdb_lib.dir/depend

