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
include CMakeFiles/memdb.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/memdb.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/memdb.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/memdb.dir/flags.make

CMakeFiles/memdb.dir/main.cpp.obj: CMakeFiles/memdb.dir/flags.make
CMakeFiles/memdb.dir/main.cpp.obj: ../main.cpp
CMakeFiles/memdb.dir/main.cpp.obj: CMakeFiles/memdb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\programacion\MemDB\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/memdb.dir/main.cpp.obj"
	C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/memdb.dir/main.cpp.obj -MF CMakeFiles\memdb.dir\main.cpp.obj.d -o CMakeFiles\memdb.dir\main.cpp.obj -c C:\programacion\MemDB\main.cpp

CMakeFiles/memdb.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/memdb.dir/main.cpp.i"
	C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\programacion\MemDB\main.cpp > CMakeFiles\memdb.dir\main.cpp.i

CMakeFiles/memdb.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/memdb.dir/main.cpp.s"
	C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\programacion\MemDB\main.cpp -o CMakeFiles\memdb.dir\main.cpp.s

CMakeFiles/memdb.dir/src/utils/DBMap.cpp.obj: CMakeFiles/memdb.dir/flags.make
CMakeFiles/memdb.dir/src/utils/DBMap.cpp.obj: ../src/utils/DBMap.cpp
CMakeFiles/memdb.dir/src/utils/DBMap.cpp.obj: CMakeFiles/memdb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\programacion\MemDB\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/memdb.dir/src/utils/DBMap.cpp.obj"
	C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/memdb.dir/src/utils/DBMap.cpp.obj -MF CMakeFiles\memdb.dir\src\utils\DBMap.cpp.obj.d -o CMakeFiles\memdb.dir\src\utils\DBMap.cpp.obj -c C:\programacion\MemDB\src\utils\DBMap.cpp

CMakeFiles/memdb.dir/src/utils/DBMap.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/memdb.dir/src/utils/DBMap.cpp.i"
	C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\programacion\MemDB\src\utils\DBMap.cpp > CMakeFiles\memdb.dir\src\utils\DBMap.cpp.i

CMakeFiles/memdb.dir/src/utils/DBMap.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/memdb.dir/src/utils/DBMap.cpp.s"
	C:\Users\polid\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.4345.21\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\programacion\MemDB\src\utils\DBMap.cpp -o CMakeFiles\memdb.dir\src\utils\DBMap.cpp.s

# Object files for target memdb
memdb_OBJECTS = \
"CMakeFiles/memdb.dir/main.cpp.obj" \
"CMakeFiles/memdb.dir/src/utils/DBMap.cpp.obj"

# External object files for target memdb
memdb_EXTERNAL_OBJECTS =

memdb.exe: CMakeFiles/memdb.dir/main.cpp.obj
memdb.exe: CMakeFiles/memdb.dir/src/utils/DBMap.cpp.obj
memdb.exe: CMakeFiles/memdb.dir/build.make
memdb.exe: CMakeFiles/memdb.dir/linklibs.rsp
memdb.exe: CMakeFiles/memdb.dir/objects1.rsp
memdb.exe: CMakeFiles/memdb.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\programacion\MemDB\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable memdb.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\memdb.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/memdb.dir/build: memdb.exe
.PHONY : CMakeFiles/memdb.dir/build

CMakeFiles/memdb.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\memdb.dir\cmake_clean.cmake
.PHONY : CMakeFiles/memdb.dir/clean

CMakeFiles/memdb.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\programacion\MemDB C:\programacion\MemDB C:\programacion\MemDB\cmake-build-debug C:\programacion\MemDB\cmake-build-debug C:\programacion\MemDB\cmake-build-debug\CMakeFiles\memdb.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/memdb.dir/depend

