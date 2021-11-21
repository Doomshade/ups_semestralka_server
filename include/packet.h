#ifndef SEMESTRALKA_PACKET_H
#define SEMESTRALKA_PACKET_H

#include "player_mngr.h"
#include "packet_registry.h"

#define MAX_MESSAGE_SIZE 50 // we don't want to let the players' messages to be too long

/**
 * The first packet that should be sent by the client, meaning
 * the client should be in the JUST_CONNECTED state. Pushes the
 * client to the LOGGED_IN state and updates the data in the player
 * arr if the client has actually reconnected
 * @param pl the player
 * @param packet the packet
 * @return 0 if everything was okay
 */
int p_hello(struct player* pl, char* packet);

/**
 * The player wants to look for a game OR he wants to leave one.
 * The client should be in the LOGGED_IN state to be added to the
 * queue, and his state is changed to QUEUE. The client's state is
 * changed to PLAY once an opponent is found.
 * The client should be in the QUEUE state to be removed from the
 * queue, and his state is changed to LOGGED_IN.
 * @param p the player
 * @param data the packet
 * @return 0 if everything was okay
 */
int p_queue(struct player* p, char* data);

/**
 * The player wants to move a piece on the g. The client should
 * be in the PLAY state. The data should always be of length 4 and
 * the data should be in format [file_from][rank_from][file_to][rank_to]
 * E.g. A2A4 E5E4
 * @param p the player
 * @param packet the packet
 * @return 0 if everything was okay
 */
int p_movepc(struct player* p, char* packet);

/**
 * The player offered a draw. The client should be in the PLAY state.
 * The data should always be of length 0 and the data should be empty.
 * @param p the player
 * @param packet the packet
 * @return 0 if everything was okay
 */
int p_offdraw(struct player* p, char* packet);

/**
 * The player resigned. The client should be in the PLAY state.
 * The data should always be of length 0 and the data should be empty.
 * This puts the player into the LOGGED_IN state.
 * @param p the player
 * @param packet the packet
 * @return 0 if everything was okay
 */
int p_resign(struct player* p, char* packet);

/**
 * The player sent some message kind of a message. The client should
 * be in the PLAY state. The max message length sent is defined
 * by MAX_MESSAGE_SIZE.
 * @param p the player
 * @param packet the packet
 * @return
 */
int p_message(struct player* p, char* packet);


#endif //SEMESTRALKA_PACKET_H
