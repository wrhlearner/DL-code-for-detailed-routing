# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn

# Include any dependencies generated for this target.
include CMakeFiles/DQN.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/DQN.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/DQN.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/DQN.dir/flags.make

CMakeFiles/DQN.dir/main.cpp.o: CMakeFiles/DQN.dir/flags.make
CMakeFiles/DQN.dir/main.cpp.o: main.cpp
CMakeFiles/DQN.dir/main.cpp.o: CMakeFiles/DQN.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/DQN.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/DQN.dir/main.cpp.o -MF CMakeFiles/DQN.dir/main.cpp.o.d -o CMakeFiles/DQN.dir/main.cpp.o -c /home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn/main.cpp

CMakeFiles/DQN.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DQN.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn/main.cpp > CMakeFiles/DQN.dir/main.cpp.i

CMakeFiles/DQN.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DQN.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn/main.cpp -o CMakeFiles/DQN.dir/main.cpp.s

# Object files for target DQN
DQN_OBJECTS = \
"CMakeFiles/DQN.dir/main.cpp.o"

# External object files for target DQN
DQN_EXTERNAL_OBJECTS =

DQN: CMakeFiles/DQN.dir/main.cpp.o
DQN: CMakeFiles/DQN.dir/build.make
DQN: env/libenv.a
DQN: /home/jborg/App/libtorch/lib/libtorch.so
DQN: /home/jborg/App/libtorch/lib/libc10.so
DQN: /home/jborg/App/libtorch/lib/libkineto.a
DQN: /home/jborg/App/libtorch/lib/libc10.so
DQN: CMakeFiles/DQN.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable DQN"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/DQN.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/DQN.dir/build: DQN
.PHONY : CMakeFiles/DQN.dir/build

CMakeFiles/DQN.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/DQN.dir/cmake_clean.cmake
.PHONY : CMakeFiles/DQN.dir/clean

CMakeFiles/DQN.dir/depend:
	cd /home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn /home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn /home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn /home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn /home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn/CMakeFiles/DQN.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/DQN.dir/depend

