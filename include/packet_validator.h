#ifndef SEMESTRALKA_PACKET_VALIDATOR_H
#define SEMESTRALKA_PACKET_VALIDATOR_H

#include <string.h>
#include "../include/packet_registry.h"

#define PACKET_MAGIC_HEADER "CHESS"
#define PACKET_ID_LENGTH 2
#define PACKET_SIZE_LENGTH 3
#define PACKET_HEADER_FORMAT "%4s%02x%03d%s"
#define PACKET_HEADER_SIZE strlen(PACKET_MAGIC_HEADER) + PACKET_ID_LENGTH + PACKET_SIZE_LENGTH
#define PACKET_OK 0
#define PACKET_NOT_YET_FULLY_BUFFERED -1
#define PACKET_INVALID_HEADER_LENGTH 1
#define PACKET_INVALID_HEADER_MAGIC 2
#define PACKET_INVALID_HEADER_ID 3
#define PACKET_INVALID_HEADER_PACKET_SIZE 4
#define PACKET_INVALID_SIZE 5

#define CLIENT_SENT_PACKET_IN_INVALID_STATE 0x69

#define PACKET_INVALID_FORMAT 1
#define PACKET_INVALID_DATA 3

void init_pvalidator();

struct packet* parse_packet(char** packet, int* erc, struct player* pl);

void free_buffers(int fd);

#endif //SEMESTRALKA_PACKET_VALIDATOR_H
