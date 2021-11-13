#include "../include/packet_registry.h"
#include "../include/packet.h"
#include <stdlib.h>
#include <stdio.h>

#define PACKET_COUNT 0x80
#define STATE_COUNT 4
packet_handle* handlers[STATE_COUNT][PACKET_COUNT] = {0};

void register_packets() {
    handlers[JUST_CONNECTED][0x00] = p_hello;
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

packet_handle* get_handler(unsigned int id, enum player_state pstate, int* erc) {
    packet_handle* handle;
    if (id >= PACKET_COUNT) {
        *erc = PACKET_ERR_INVALID_ID;
        return NULL;
    }

    // check if the state count is even right
    if (pstate >= STATE_COUNT) {
        printf("Reached a state that does not exist!\n");
        *erc = PACKET_ERR_STATE_OUT_OF_BOUNDS;
        return NULL;
    }
    handle = handlers[pstate][id];
    if (!handle) {
        *erc = PACKET_ERR_INVALID_ID;
        return NULL;
    }
    *erc = 0;
    return handle;
}