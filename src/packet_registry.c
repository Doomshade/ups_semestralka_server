#include "../include/packet_registry.h"
#include "../include/packet_in.h"
#include <stdlib.h>

#define PACKET_COUNT 0xA0
packet_handle* handlers[PACKET_COUNT] = {0};

void register_packets() {
    handlers[0x00] = p_hello;
}

struct packet* create_packet(unsigned int id, unsigned int len, char* data) {
    struct packet* p = malloc(sizeof(struct packet));
    if (p == NULL) {
        return NULL;
    }
    p->id = id;
    p->len = len;
    p->data = data;
    return p;
}

packet_handle* get_handler(unsigned int id) {
    if (id >= PACKET_COUNT){
        return NULL;
    }
    return handlers[id];
}