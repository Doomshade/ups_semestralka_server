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

#define PACKER_ERR_INVALID_CLIENT_STATE 0x69

#define PACKET_INVALID_FORMAT 1
#define PACKET_INVALID_DATA 3

/**
 * Initializes the packet validator
 */
void init_pvalidator();

/**
 * Attempts to parse the packet sent by the player. Note that the
 * pointer is shifted, if packet such as "CHESS00001ACHESS01" is sent,
 * the pointer will point to the next packet - "CHESS01"
 * @param packet the packet
 * @param erc the error code, see definitions starting with PACKET_
 * @param pl the player
 * @return the packet once the packet is parsed
 */
struct packet* parse_packet(char** packet, int* erc, struct player* pl);

/**
 * Frees the header and data buffers of a client
 * @param fd the client's file descriptor
 */
void free_buffers(int fd);

#endif //SEMESTRALKA_PACKET_VALIDATOR_H
