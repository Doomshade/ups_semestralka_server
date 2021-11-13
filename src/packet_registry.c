#include "../include/packet_registry.h"
#include "../include/packet.h"
#include "../include/packet_validator.h"
#include <stdlib.h>
#include <stdio.h>

#define PACKET_COUNT 0x80
#define STATE_COUNT 4
packet_handle* handlers[STATE_COUNT][PACKET_COUNT] = {0};

void register_packets() {
    handlers[JUST_CONNECTED][HELLO_IN] = p_hello;
    handlers[LOGGED_IN][QUEUE_IN] = p_queue;
    handlers[QUEUE][QUEUE_IN] = p_queue;
    handlers[PLAY][MOVE_IN] = p_movepc;
    handlers[PLAY][DRAW_OFFER_IN] = p_offdraw;
    handlers[PLAY][RESIGN_IN] = p_resign;
}

int send_raw(struct player* p, char* buf, unsigned long* len) {
    unsigned long total = 0; // how many bytes we've sent
    unsigned long bytesleft = *len; // how many we have left to send
    long n;
    while (total < *len) {
        n = send(p->fd, buf + total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    *len = total; // return number actually sent here
    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

int send_packet(struct player* pl, struct packet* pc) {
    int ret;
    char* s = malloc(PACKET_HEADER_SIZE + strlen(pc->data) + 1); // the data we send
    unsigned long len; // the length of the data

    sprintf(s, PACKET_HEADER_FORMAT, PACKET_MAGIC_HEADER, pc->id, pc->len, pc->data);
    len = strlen(s);
    ret = send_raw(pl, s, &len);
    free(s);
    //sscanf(s, PACKET_MAGIC_HEADER, pc->id, pc->len, pc->data);
    return ret;
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