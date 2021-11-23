#ifndef SEMESTRALKA_PACKET_HANDLER_H
#define SEMESTRALKA_PACKET_HANDLER_H

#include "packet_registry.h"
#define PLAYER_INVALID_STATE 0x24

/**
 * Handles the packet sent by the player
 * @param pl the player
 * @param pckt the packet
 * @return 0 if everything went alright
 */
int handle_packet(struct player* pl, unsigned int id, char* data);

#endif //SEMESTRALKA_PACKET_HANDLER_H
