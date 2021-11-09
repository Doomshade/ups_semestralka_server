#ifndef SEMESTRALKA_PACKET_IN_H
#define SEMESTRALKA_PACKET_IN_H

int p_hello(int fd, char* packet);

int p_findgm(int fd, char* data);

int p_movepc(int fd, char* data);

int p_offdraw(int fd, char* data);

int p_resign(int fd, char* data);


#endif //SEMESTRALKA_PACKET_IN_H
