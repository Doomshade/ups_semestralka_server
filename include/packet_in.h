#ifndef SEMESTRALKA_PACKET_IN_H
#define SEMESTRALKA_PACKET_IN_H
#include "../include/game_mngr.h"

int p_hello(struct player* p, char* packet);

int p_findgm(int fd, char* data);

int p_movepc(int fd, char* data);

int p_offdraw(int fd, char* data);

int p_resign(int fd, char* data);


#endif //SEMESTRALKA_PACKET_IN_H
