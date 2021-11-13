#ifndef SEMESTRALKA_PACKET_REGISTRY_H
#define SEMESTRALKA_PACKET_REGISTRY_H

#include "game_mngr.h"
#include "player_mngr.h"
#define PACKET_ERR_INVALID_ID 0x11
#define PACKET_ERR_STATE_OUT_OF_BOUNDS 0x12

struct packet {
    /**
     * The packet ID
     */
    unsigned int id;
    /**
     * The packet length/size
     */
    unsigned int len;
    /**
     * The data/payload of the packet
     */
    char* data;
};

/**
 * The packet handling function
 */
typedef int packet_handle(struct player* fd, char* data);

struct packet* create_packet(unsigned int id, unsigned int len, char* data);

/**
 * Gets the packet handler based on the packet ID
 * @param id the packet ID
 * @return
 */
packet_handle* get_handler(unsigned int id, enum player_state pstate, int* erc);

/**
 * Registers the packets in the memory
 */
void register_packets();

#endif //SEMESTRALKA_PACKET_REGISTRY_H
