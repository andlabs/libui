# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kusti8/Documents/Github/libui

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kusti8/Documents/Github/libui/build

# Include any dependencies generated for this target.
include examples/CMakeFiles/layout-test.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/layout-test.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/layout-test.dir/flags.make

examples/CMakeFiles/layout-test.dir/layout-test/main.c.o: examples/CMakeFiles/layout-test.dir/flags.make
examples/CMakeFiles/layout-test.dir/layout-test/main.c.o: ../examples/layout-test/main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kusti8/Documents/Github/libui/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object examples/CMakeFiles/layout-test.dir/layout-test/main.c.o"
	cd /home/kusti8/Documents/Github/libui/build/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/layout-test.dir/layout-test/main.c.o   -c /home/kusti8/Documents/Github/libui/examples/layout-test/main.c

examples/CMakeFiles/layout-test.dir/layout-test/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/layout-test.dir/layout-test/main.c.i"
	cd /home/kusti8/Documents/Github/libui/build/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/kusti8/Documents/Github/libui/examples/layout-test/main.c > CMakeFiles/layout-test.dir/layout-test/main.c.i

examples/CMakeFiles/layout-test.dir/layout-test/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/layout-test.dir/layout-test/main.c.s"
	cd /home/kusti8/Documents/Github/libui/build/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/kusti8/Documents/Github/libui/examples/layout-test/main.c -o CMakeFiles/layout-test.dir/layout-test/main.c.s

examples/CMakeFiles/layout-test.dir/layout-test/main.c.o.requires:

.PHONY : examples/CMakeFiles/layout-test.dir/layout-test/main.c.o.requires

examples/CMakeFiles/layout-test.dir/layout-test/main.c.o.provides: examples/CMakeFiles/layout-test.dir/layout-test/main.c.o.requires
	$(MAKE) -f examples/CMakeFiles/layout-test.dir/build.make examples/CMakeFiles/layout-test.dir/layout-test/main.c.o.provides.build
.PHONY : examples/CMakeFiles/layout-test.dir/layout-test/main.c.o.provides

examples/CMakeFiles/layout-test.dir/layout-test/main.c.o.provides.build: examples/CMakeFiles/layout-test.dir/layout-test/main.c.o


# Object files for target layout-test
layout__test_OBJECTS = \
"CMakeFiles/layout-test.dir/layout-test/main.c.o"

# External object files for target layout-test
layout__test_EXTERNAL_OBJECTS =

out/layout-test: examples/CMakeFiles/layout-test.dir/layout-test/main.c.o
out/layout-test: examples/CMakeFiles/layout-test.dir/build.make
out/layout-test: out/libui.so.0
out/layout-test: examples/CMakeFiles/layout-test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kusti8/Documents/Github/libui/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ../out/layout-test"
	cd /home/kusti8/Documents/Github/libui/build/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/layout-test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/layout-test.dir/build: out/layout-test

.PHONY : examples/CMakeFiles/layout-test.dir/build

examples/CMakeFiles/layout-test.dir/requires: examples/CMakeFiles/layout-test.dir/layout-test/main.c.o.requires

.PHONY : examples/CMakeFiles/layout-test.dir/requires

examples/CMakeFiles/layout-test.dir/clean:
	cd /home/kusti8/Documents/Github/libui/build/examples && $(CMAKE_COMMAND) -P CMakeFiles/layout-test.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/layout-test.dir/clean

examples/CMakeFiles/layout-test.dir/depend:
	cd /home/kusti8/Documents/Github/libui/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kusti8/Documents/Github/libui /home/kusti8/Documents/Github/libui/examples /home/kusti8/Documents/Github/libui/build /home/kusti8/Documents/Github/libui/build/examples /home/kusti8/Documents/Github/libui/build/examples/CMakeFiles/layout-test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/layout-test.dir/depend

