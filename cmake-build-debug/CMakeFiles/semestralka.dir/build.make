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
CMAKE_COMMAND = /home/doomshade/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/212.5457.51/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/doomshade/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/212.5457.51/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/doomshade/CLionProjects/semestralka

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/doomshade/CLionProjects/semestralka/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/semestralka.dir/depend.make
# Include the progress variables for this target.
include CMakeFiles/semestralka.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/semestralka.dir/flags.make

CMakeFiles/semestralka.dir/src/main.c.o: CMakeFiles/semestralka.dir/flags.make
CMakeFiles/semestralka.dir/src/main.c.o: ../src/main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/doomshade/CLionProjects/semestralka/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/semestralka.dir/src/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/semestralka.dir/src/main.c.o -c /home/doomshade/CLionProjects/semestralka/src/main.c

CMakeFiles/semestralka.dir/src/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/semestralka.dir/src/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/doomshade/CLionProjects/semestralka/src/main.c > CMakeFiles/semestralka.dir/src/main.c.i

CMakeFiles/semestralka.dir/src/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/semestralka.dir/src/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/doomshade/CLionProjects/semestralka/src/main.c -o CMakeFiles/semestralka.dir/src/main.c.s

CMakeFiles/semestralka.dir/src/packet_handler.c.o: CMakeFiles/semestralka.dir/flags.make
CMakeFiles/semestralka.dir/src/packet_handler.c.o: ../src/packet_handler.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/doomshade/CLionProjects/semestralka/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/semestralka.dir/src/packet_handler.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/semestralka.dir/src/packet_handler.c.o -c /home/doomshade/CLionProjects/semestralka/src/packet_handler.c

CMakeFiles/semestralka.dir/src/packet_handler.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/semestralka.dir/src/packet_handler.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/doomshade/CLionProjects/semestralka/src/packet_handler.c > CMakeFiles/semestralka.dir/src/packet_handler.c.i

CMakeFiles/semestralka.dir/src/packet_handler.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/semestralka.dir/src/packet_handler.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/doomshade/CLionProjects/semestralka/src/packet_handler.c -o CMakeFiles/semestralka.dir/src/packet_handler.c.s

CMakeFiles/semestralka.dir/src/packet_validator.c.o: CMakeFiles/semestralka.dir/flags.make
CMakeFiles/semestralka.dir/src/packet_validator.c.o: ../src/packet_validator.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/doomshade/CLionProjects/semestralka/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/semestralka.dir/src/packet_validator.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/semestralka.dir/src/packet_validator.c.o -c /home/doomshade/CLionProjects/semestralka/src/packet_validator.c

CMakeFiles/semestralka.dir/src/packet_validator.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/semestralka.dir/src/packet_validator.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/doomshade/CLionProjects/semestralka/src/packet_validator.c > CMakeFiles/semestralka.dir/src/packet_validator.c.i

CMakeFiles/semestralka.dir/src/packet_validator.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/semestralka.dir/src/packet_validator.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/doomshade/CLionProjects/semestralka/src/packet_validator.c -o CMakeFiles/semestralka.dir/src/packet_validator.c.s

CMakeFiles/semestralka.dir/src/server.c.o: CMakeFiles/semestralka.dir/flags.make
CMakeFiles/semestralka.dir/src/server.c.o: ../src/server.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/doomshade/CLionProjects/semestralka/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/semestralka.dir/src/server.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/semestralka.dir/src/server.c.o -c /home/doomshade/CLionProjects/semestralka/src/server.c

CMakeFiles/semestralka.dir/src/server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/semestralka.dir/src/server.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/doomshade/CLionProjects/semestralka/src/server.c > CMakeFiles/semestralka.dir/src/server.c.i

CMakeFiles/semestralka.dir/src/server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/semestralka.dir/src/server.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/doomshade/CLionProjects/semestralka/src/server.c -o CMakeFiles/semestralka.dir/src/server.c.s

CMakeFiles/semestralka.dir/src/packet.c.o: CMakeFiles/semestralka.dir/flags.make
CMakeFiles/semestralka.dir/src/packet.c.o: ../src/packet.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/doomshade/CLionProjects/semestralka/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/semestralka.dir/src/packet.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/semestralka.dir/src/packet.c.o -c /home/doomshade/CLionProjects/semestralka/src/packet.c

CMakeFiles/semestralka.dir/src/packet.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/semestralka.dir/src/packet.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/doomshade/CLionProjects/semestralka/src/packet.c > CMakeFiles/semestralka.dir/src/packet.c.i

CMakeFiles/semestralka.dir/src/packet.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/semestralka.dir/src/packet.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/doomshade/CLionProjects/semestralka/src/packet.c -o CMakeFiles/semestralka.dir/src/packet.c.s

CMakeFiles/semestralka.dir/src/game_mngr.c.o: CMakeFiles/semestralka.dir/flags.make
CMakeFiles/semestralka.dir/src/game_mngr.c.o: ../src/game_mngr.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/doomshade/CLionProjects/semestralka/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object CMakeFiles/semestralka.dir/src/game_mngr.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/semestralka.dir/src/game_mngr.c.o -c /home/doomshade/CLionProjects/semestralka/src/game_mngr.c

CMakeFiles/semestralka.dir/src/game_mngr.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/semestralka.dir/src/game_mngr.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/doomshade/CLionProjects/semestralka/src/game_mngr.c > CMakeFiles/semestralka.dir/src/game_mngr.c.i

CMakeFiles/semestralka.dir/src/game_mngr.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/semestralka.dir/src/game_mngr.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/doomshade/CLionProjects/semestralka/src/game_mngr.c -o CMakeFiles/semestralka.dir/src/game_mngr.c.s

CMakeFiles/semestralka.dir/src/packet_registry.c.o: CMakeFiles/semestralka.dir/flags.make
CMakeFiles/semestralka.dir/src/packet_registry.c.o: ../src/packet_registry.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/doomshade/CLionProjects/semestralka/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object CMakeFiles/semestralka.dir/src/packet_registry.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/semestralka.dir/src/packet_registry.c.o -c /home/doomshade/CLionProjects/semestralka/src/packet_registry.c

CMakeFiles/semestralka.dir/src/packet_registry.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/semestralka.dir/src/packet_registry.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/doomshade/CLionProjects/semestralka/src/packet_registry.c > CMakeFiles/semestralka.dir/src/packet_registry.c.i

CMakeFiles/semestralka.dir/src/packet_registry.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/semestralka.dir/src/packet_registry.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/doomshade/CLionProjects/semestralka/src/packet_registry.c -o CMakeFiles/semestralka.dir/src/packet_registry.c.s

CMakeFiles/semestralka.dir/src/player_mngr.c.o: CMakeFiles/semestralka.dir/flags.make
CMakeFiles/semestralka.dir/src/player_mngr.c.o: ../src/player_mngr.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/doomshade/CLionProjects/semestralka/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object CMakeFiles/semestralka.dir/src/player_mngr.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/semestralka.dir/src/player_mngr.c.o -c /home/doomshade/CLionProjects/semestralka/src/player_mngr.c

CMakeFiles/semestralka.dir/src/player_mngr.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/semestralka.dir/src/player_mngr.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/doomshade/CLionProjects/semestralka/src/player_mngr.c > CMakeFiles/semestralka.dir/src/player_mngr.c.i

CMakeFiles/semestralka.dir/src/player_mngr.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/semestralka.dir/src/player_mngr.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/doomshade/CLionProjects/semestralka/src/player_mngr.c -o CMakeFiles/semestralka.dir/src/player_mngr.c.s

# Object files for target semestralka
semestralka_OBJECTS = \
"CMakeFiles/semestralka.dir/src/main.c.o" \
"CMakeFiles/semestralka.dir/src/packet_handler.c.o" \
"CMakeFiles/semestralka.dir/src/packet_validator.c.o" \
"CMakeFiles/semestralka.dir/src/server.c.o" \
"CMakeFiles/semestralka.dir/src/packet.c.o" \
"CMakeFiles/semestralka.dir/src/game_mngr.c.o" \
"CMakeFiles/semestralka.dir/src/packet_registry.c.o" \
"CMakeFiles/semestralka.dir/src/player_mngr.c.o"

# External object files for target semestralka
semestralka_EXTERNAL_OBJECTS =

semestralka: CMakeFiles/semestralka.dir/src/main.c.o
semestralka: CMakeFiles/semestralka.dir/src/packet_handler.c.o
semestralka: CMakeFiles/semestralka.dir/src/packet_validator.c.o
semestralka: CMakeFiles/semestralka.dir/src/server.c.o
semestralka: CMakeFiles/semestralka.dir/src/packet.c.o
semestralka: CMakeFiles/semestralka.dir/src/game_mngr.c.o
semestralka: CMakeFiles/semestralka.dir/src/packet_registry.c.o
semestralka: CMakeFiles/semestralka.dir/src/player_mngr.c.o
semestralka: CMakeFiles/semestralka.dir/build.make
semestralka: CMakeFiles/semestralka.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/doomshade/CLionProjects/semestralka/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Linking C executable semestralka"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/semestralka.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/semestralka.dir/build: semestralka
.PHONY : CMakeFiles/semestralka.dir/build

CMakeFiles/semestralka.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/semestralka.dir/cmake_clean.cmake
.PHONY : CMakeFiles/semestralka.dir/clean

CMakeFiles/semestralka.dir/depend:
	cd /home/doomshade/CLionProjects/semestralka/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/doomshade/CLionProjects/semestralka /home/doomshade/CLionProjects/semestralka /home/doomshade/CLionProjects/semestralka/cmake-build-debug /home/doomshade/CLionProjects/semestralka/cmake-build-debug /home/doomshade/CLionProjects/semestralka/cmake-build-debug/CMakeFiles/semestralka.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/semestralka.dir/depend

