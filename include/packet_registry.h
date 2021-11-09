#ifndef SEMESTRALKA_PACKET_REGISTRY_H
#define SEMESTRALKA_PACKET_REGISTRY_H
#include "game_mngr.h"

struct packet {
    unsigned int id;
    unsigned int len;
    char* data;
};

typedef int packet_handle(struct player *fd, char* data);

struct packet* create_packet(unsigned int id, unsigned int len, char* data);

packet_handle* get_handler(unsigned int id);

void register_packets();

#endif //SEMESTRALKA_PACKET_REGISTRY_H
