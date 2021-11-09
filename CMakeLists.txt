cmake_minimum_required(VERSION 3.20)
project(semestralka C)

set(CMAKE_C_STANDARD 11)

add_executable(semestralka src/main.c src/packet_handler.c include/packet_handler.h src/packet_validator.c include/packet_validator.h src/server.c include/server.h src/packet_in.c include/packet_in.h src/game_mngr.c include/game_mngr.h src/packet_registry.c include/packet_registry.h src/player_mngr.c include/player_mngr.h)
