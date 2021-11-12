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

int p_findgm(int fd, char* data);

int p_movepc(int fd, char* data);

int p_offdraw(int fd, char* data);

int p_resign(int fd, char* data);


#endif //SEMESTRALKA_PACKET_H
