cmake_minimum_required(VERSION 3.20)
project(semestralka C)

set(CMAKE_C_STANDARD 11)

add_executable(semestralka src/main.c src/packet_handler.c include/packet_handler.h src/packet_validator.c include/packet_validator.h src/connection.c include/connection.h src/packet.c include/packet.h)
