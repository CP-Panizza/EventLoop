# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.11

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
CMAKE_COMMAND = /home/cmj/cmake-3.11.4/bin/cmake

# The command to remove a file.
RM = /home/cmj/cmake-3.11.4/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cmj/桌面/EventLoop

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cmj/桌面/EventLoop/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/EventLoop.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/EventLoop.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/EventLoop.dir/flags.make

CMakeFiles/EventLoop.dir/main.cpp.o: CMakeFiles/EventLoop.dir/flags.make
CMakeFiles/EventLoop.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cmj/桌面/EventLoop/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/EventLoop.dir/main.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/EventLoop.dir/main.cpp.o -c /home/cmj/桌面/EventLoop/main.cpp

CMakeFiles/EventLoop.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/EventLoop.dir/main.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cmj/桌面/EventLoop/main.cpp > CMakeFiles/EventLoop.dir/main.cpp.i

CMakeFiles/EventLoop.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/EventLoop.dir/main.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cmj/桌面/EventLoop/main.cpp -o CMakeFiles/EventLoop.dir/main.cpp.s

# Object files for target EventLoop
EventLoop_OBJECTS = \
"CMakeFiles/EventLoop.dir/main.cpp.o"

# External object files for target EventLoop
EventLoop_EXTERNAL_OBJECTS =

EventLoop: CMakeFiles/EventLoop.dir/main.cpp.o
EventLoop: CMakeFiles/EventLoop.dir/build.make
EventLoop: CMakeFiles/EventLoop.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cmj/桌面/EventLoop/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable EventLoop"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/EventLoop.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/EventLoop.dir/build: EventLoop

.PHONY : CMakeFiles/EventLoop.dir/build

CMakeFiles/EventLoop.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/EventLoop.dir/cmake_clean.cmake
.PHONY : CMakeFiles/EventLoop.dir/clean

CMakeFiles/EventLoop.dir/depend:
	cd /home/cmj/桌面/EventLoop/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cmj/桌面/EventLoop /home/cmj/桌面/EventLoop /home/cmj/桌面/EventLoop/cmake-build-debug /home/cmj/桌面/EventLoop/cmake-build-debug /home/cmj/桌面/EventLoop/cmake-build-debug/CMakeFiles/EventLoop.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/EventLoop.dir/depend
