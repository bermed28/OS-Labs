# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050/cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/lab4_4050.dir/depend.make
# Include the progress variables for this target.
include CMakeFiles/lab4_4050.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lab4_4050.dir/flags.make

CMakeFiles/lab4_4050.dir/ex3.c.o: CMakeFiles/lab4_4050.dir/flags.make
CMakeFiles/lab4_4050.dir/ex3.c.o: ../ex3.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/lab4_4050.dir/ex3.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/lab4_4050.dir/ex3.c.o -c "/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050/ex3.c"

CMakeFiles/lab4_4050.dir/ex3.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lab4_4050.dir/ex3.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050/ex3.c" > CMakeFiles/lab4_4050.dir/ex3.c.i

CMakeFiles/lab4_4050.dir/ex3.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lab4_4050.dir/ex3.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050/ex3.c" -o CMakeFiles/lab4_4050.dir/ex3.c.s

# Object files for target lab4_4050
lab4_4050_OBJECTS = \
"CMakeFiles/lab4_4050.dir/ex3.c.o"

# External object files for target lab4_4050
lab4_4050_EXTERNAL_OBJECTS =

lab4_4050: CMakeFiles/lab4_4050.dir/ex3.c.o
lab4_4050: CMakeFiles/lab4_4050.dir/build.make
lab4_4050: CMakeFiles/lab4_4050.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable lab4_4050"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lab4_4050.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lab4_4050.dir/build: lab4_4050
.PHONY : CMakeFiles/lab4_4050.dir/build

CMakeFiles/lab4_4050.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/lab4_4050.dir/cmake_clean.cmake
.PHONY : CMakeFiles/lab4_4050.dir/clean

CMakeFiles/lab4_4050.dir/depend:
	cd "/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050" "/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050" "/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050/cmake-build-debug" "/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050/cmake-build-debug" "/Users/bermed28/Desktop/CIIC4050/Labs/Lab 4/lab4_4050/cmake-build-debug/CMakeFiles/lab4_4050.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/lab4_4050.dir/depend

