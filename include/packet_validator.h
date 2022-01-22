#ifndef SEMESTRALKA_PACKET_VALIDATOR_H
#define SEMESTRALKA_PACKET_VALIDATOR_H

#include <string.h>
#include "packet_registry.h"

#define PACKET_MAGIC_HEADER "CHESS"
#define PACKET_ID_LENGTH 2
#define PACKET_SIZE_LENGTH 3
// (CHESS)(00-FF)(000-999)(<payload>)
#define PACKET_HEADER_FORMAT "%5s%02x%03lu%s"
#define PACKET_HEADER_SIZE strlen(PACKET_MAGIC_HEADER) + PACKET_ID_LENGTH + PACKET_SIZE_LENGTH
#define PVAL_PACKET_OK 0
#define PVAL_PACKET_NOT_YET_FULLY_BUFFERED -1
#define PVAL_PACKET_INVALID_HEADER_LENGTH 1
#define PVAL_PACKET_INVALID_HEADER_MAGIC 2
#define PVAL_PACKET_INVALID_HEADER_ID 3
#define PVAL_PACKET_INVALID_HEADER_PACKET_SIZE 4

#define PVAL_PACKET_INVALID_DATA 3

/**
 * Initializes the packet validator
 */
void init_pval(unsigned max_players);

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
