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
CMAKE_SOURCE_DIR = /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test

# Include any dependencies generated for this target.
include CMakeFiles/rtpall.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rtpall.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rtpall.dir/flags.make

CMakeFiles/rtpall.dir/src/rtp.cpp.o: CMakeFiles/rtpall.dir/flags.make
CMakeFiles/rtpall.dir/src/rtp.cpp.o: src/rtp.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/rtpall.dir/src/rtp.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rtpall.dir/src/rtp.cpp.o -c /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test/src/rtp.cpp

CMakeFiles/rtpall.dir/src/rtp.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rtpall.dir/src/rtp.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test/src/rtp.cpp > CMakeFiles/rtpall.dir/src/rtp.cpp.i

CMakeFiles/rtpall.dir/src/rtp.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rtpall.dir/src/rtp.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test/src/rtp.cpp -o CMakeFiles/rtpall.dir/src/rtp.cpp.s

CMakeFiles/rtpall.dir/src/util.cpp.o: CMakeFiles/rtpall.dir/flags.make
CMakeFiles/rtpall.dir/src/util.cpp.o: src/util.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/rtpall.dir/src/util.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rtpall.dir/src/util.cpp.o -c /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test/src/util.cpp

CMakeFiles/rtpall.dir/src/util.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rtpall.dir/src/util.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test/src/util.cpp > CMakeFiles/rtpall.dir/src/util.cpp.i

CMakeFiles/rtpall.dir/src/util.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rtpall.dir/src/util.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test/src/util.cpp -o CMakeFiles/rtpall.dir/src/util.cpp.s

# Object files for target rtpall
rtpall_OBJECTS = \
"CMakeFiles/rtpall.dir/src/rtp.cpp.o" \
"CMakeFiles/rtpall.dir/src/util.cpp.o"

# External object files for target rtpall
rtpall_EXTERNAL_OBJECTS =

librtpall.a: CMakeFiles/rtpall.dir/src/rtp.cpp.o
librtpall.a: CMakeFiles/rtpall.dir/src/util.cpp.o
librtpall.a: CMakeFiles/rtpall.dir/build.make
librtpall.a: CMakeFiles/rtpall.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library librtpall.a"
	$(CMAKE_COMMAND) -P CMakeFiles/rtpall.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rtpall.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rtpall.dir/build: librtpall.a

.PHONY : CMakeFiles/rtpall.dir/build

CMakeFiles/rtpall.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/rtpall.dir/cmake_clean.cmake
.PHONY : CMakeFiles/rtpall.dir/clean

CMakeFiles/rtpall.dir/depend:
	cd /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test /home/fxh/Computer-Network/Lab2/lab2-rtp-Naivexc/Lab2-RTP-Test/CMakeFiles/rtpall.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rtpall.dir/depend
