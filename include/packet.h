#ifndef SEMESTRALKA_PACKET_H
#define SEMESTRALKA_PACKET_H

#include "../include/player_mngr.h"

#define PACKET_IN_OFFSET 0x80
#define PACKET_IN(out) out + PACKET_IN_OFFSET
#define PACKET_OUT(offset, id) offset + id
#define JUST_CONNECTED_P_OFFSET 0x00
#define LOGGED_IN_P_OFFSET 0x20
#define QUEUE_P_OFFSET 0x40
#define PLAY_P_OFFSET 0x60

enum just_connected_p {
    HELLO_IN = PACKET_OUT(JUST_CONNECTED_P_OFFSET, 0),
    HELLO_OUT = PACKET_IN(HELLO_IN),
};

enum logged_in_p {
    QUEUE_IN = PACKET_OUT(LOGGED_IN_P_OFFSET, 0),
    QUEUE_OUT = PACKET_IN(QUEUE_IN)
};

enum queue_p {
    CONNECT_IN = PACKET_OUT(QUEUE_P_OFFSET, 0),
    CONNECT_OUT = PACKET_IN(CONNECT_IN)
};

enum play_p {
    MOVE_IN = PACKET_OUT(PLAY_P_OFFSET, 0),
    MOVE_OUT = PACKET_IN(MOVE_IN)
};

/**
 * The first packet that should be sent by the client, meaning
 * the client should be in the JUST_CONNECTED state. Pushes the
 * client to the LOGGED_IN state and updates the data in the player
 * arr if the client has actually reconnected
 * @param p the player
 * @param packet the packet
 * @return 0 if everything was okay
 */
int p_hello(struct player* p, char* packet);

/**
 * The player wants to look for a game, we put him in a queue. The
 * client should be in the LOGGED_IN state and is updated once an
 * opponent is found
 * @param p the player
 * @param packet the packet
 * @return 0 if everything was okay
 */
int p_findgm(struct player* p, char* packet);

/**
 * The player wants to move a piece on the board. The client should
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


#endif //SEMESTRALKA_PACKET_H
