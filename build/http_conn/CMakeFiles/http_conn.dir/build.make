# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /home/lin/CPP/WebServer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lin/CPP/WebServer/build

# Include any dependencies generated for this target.
include http_conn/CMakeFiles/http_conn.dir/depend.make

# Include the progress variables for this target.
include http_conn/CMakeFiles/http_conn.dir/progress.make

# Include the compile flags for this target's objects.
include http_conn/CMakeFiles/http_conn.dir/flags.make

http_conn/CMakeFiles/http_conn.dir/http_conn.cpp.o: http_conn/CMakeFiles/http_conn.dir/flags.make
http_conn/CMakeFiles/http_conn.dir/http_conn.cpp.o: ../http_conn/http_conn.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lin/CPP/WebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object http_conn/CMakeFiles/http_conn.dir/http_conn.cpp.o"
	cd /home/lin/CPP/WebServer/build/http_conn && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/http_conn.dir/http_conn.cpp.o -c /home/lin/CPP/WebServer/http_conn/http_conn.cpp

http_conn/CMakeFiles/http_conn.dir/http_conn.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/http_conn.dir/http_conn.cpp.i"
	cd /home/lin/CPP/WebServer/build/http_conn && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lin/CPP/WebServer/http_conn/http_conn.cpp > CMakeFiles/http_conn.dir/http_conn.cpp.i

http_conn/CMakeFiles/http_conn.dir/http_conn.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/http_conn.dir/http_conn.cpp.s"
	cd /home/lin/CPP/WebServer/build/http_conn && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lin/CPP/WebServer/http_conn/http_conn.cpp -o CMakeFiles/http_conn.dir/http_conn.cpp.s

# Object files for target http_conn
http_conn_OBJECTS = \
"CMakeFiles/http_conn.dir/http_conn.cpp.o"

# External object files for target http_conn
http_conn_EXTERNAL_OBJECTS =

../lib/libhttp_conn.so: http_conn/CMakeFiles/http_conn.dir/http_conn.cpp.o
../lib/libhttp_conn.so: http_conn/CMakeFiles/http_conn.dir/build.make
../lib/libhttp_conn.so: http_conn/CMakeFiles/http_conn.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lin/CPP/WebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library ../../lib/libhttp_conn.so"
	cd /home/lin/CPP/WebServer/build/http_conn && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/http_conn.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
http_conn/CMakeFiles/http_conn.dir/build: ../lib/libhttp_conn.so

.PHONY : http_conn/CMakeFiles/http_conn.dir/build

http_conn/CMakeFiles/http_conn.dir/clean:
	cd /home/lin/CPP/WebServer/build/http_conn && $(CMAKE_COMMAND) -P CMakeFiles/http_conn.dir/cmake_clean.cmake
.PHONY : http_conn/CMakeFiles/http_conn.dir/clean

http_conn/CMakeFiles/http_conn.dir/depend:
	cd /home/lin/CPP/WebServer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lin/CPP/WebServer /home/lin/CPP/WebServer/http_conn /home/lin/CPP/WebServer/build /home/lin/CPP/WebServer/build/http_conn /home/lin/CPP/WebServer/build/http_conn/CMakeFiles/http_conn.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : http_conn/CMakeFiles/http_conn.dir/depend

