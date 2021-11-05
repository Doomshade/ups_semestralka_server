#ifndef SEMESTRALKA_PACKET_HANDLER_H
#define SEMESTRALKA_PACKET_HANDLER_H
#define PACKET_OK 0
#define PACKET_INVALID_HEADER 1
#define PACKET_INVALID_FORMAT 2
#define PACKET_INVALID_DATA 3

#include "../include/packet.h"

int (* packet_handler(char* data));

int handle_packet(char* packet);

#endif //SEMESTRALKA_PACKET_HANDLER_H
