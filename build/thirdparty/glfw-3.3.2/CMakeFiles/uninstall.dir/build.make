# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

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
CMAKE_COMMAND = /opt/homebrew/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/Users/manveetmandal/Desktop/CPSC 589/Project"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/Users/manveetmandal/Desktop/CPSC 589/Project/build"

# Utility rule file for uninstall.

# Include any custom commands dependencies for this target.
include thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/compiler_depend.make

# Include the progress variables for this target.
include thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/progress.make

thirdparty/glfw-3.3.2/CMakeFiles/uninstall:
	cd "/Users/manveetmandal/Desktop/CPSC 589/Project/build/thirdparty/glfw-3.3.2" && /opt/homebrew/bin/cmake -P /Users/manveetmandal/Desktop/CPSC\ 589/Project/build/thirdparty/glfw-3.3.2/cmake_uninstall.cmake

thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/codegen:
.PHONY : thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/codegen

uninstall: thirdparty/glfw-3.3.2/CMakeFiles/uninstall
uninstall: thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/build.make
.PHONY : uninstall

# Rule to build all files generated by this target.
thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/build: uninstall
.PHONY : thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/build

thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/clean:
	cd "/Users/manveetmandal/Desktop/CPSC 589/Project/build/thirdparty/glfw-3.3.2" && $(CMAKE_COMMAND) -P CMakeFiles/uninstall.dir/cmake_clean.cmake
.PHONY : thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/clean

thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/depend:
	cd "/Users/manveetmandal/Desktop/CPSC 589/Project/build" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/Users/manveetmandal/Desktop/CPSC 589/Project" "/Users/manveetmandal/Desktop/CPSC 589/Project/thirdparty/glfw-3.3.2" "/Users/manveetmandal/Desktop/CPSC 589/Project/build" "/Users/manveetmandal/Desktop/CPSC 589/Project/build/thirdparty/glfw-3.3.2" "/Users/manveetmandal/Desktop/CPSC 589/Project/build/thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/DependInfo.cmake" "--color=$(COLOR)"
.PHONY : thirdparty/glfw-3.3.2/CMakeFiles/uninstall.dir/depend

