cmake_minimum_required(VERSION 3.20)
project(semestralka C)

set(CMAKE_C_STANDARD 99)

add_executable(semestralka src/main.c src/packet_handler.c include/packet_handler.h src/packet_validator.c include/packet_validator.h src/server.c include/server.h src/packet.c include/packet.h src/game_mngr.c include/game_mngr.h src/packet_registry.c include/packet_registry.h src/player_mngr.c include/player_mngr.h src/queue_mngr.c include/queue_mngr.h src/chesspiece.c include/chesspiece.h)
target_link_libraries(semestralka m)
