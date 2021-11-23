#include "../include/packet_registry.h"
#include "../include/packet.h"
#include "../include/packet_validator.h"
#include <stdlib.h>
#include <stdio.h>

// the amount of IN packets = (0-PACKET_OUT_OFFSET)
#define PACKET_COUNT PACKET_OUT_OFFSET
#define STATE_COUNT 4
// I don't calloc this because callocing matrices is hard
packet_handle* packet_handlers[STATE_COUNT][PACKET_COUNT] = {0};
bool registered = false;

/**
 * Frees the packet from the memory
 * @param pc the packet
 */
void free_packet(struct packet* pc);

void init_preg() {
    if (registered) {
        return;
    }
    //[STATE_COUNT][PACKET_COUNT]
    printf("Initializing packet registry...\n");
    packet_handlers[JUST_CONNECTED][HELLO_IN] = p_hello;
    packet_handlers[LOGGED_IN][QUEUE_IN] = p_queue;
    packet_handlers[QUEUE][QUEUE_IN] = p_queue;
    packet_handlers[PLAY][MOVE_IN] = p_movepc;
    packet_handlers[PLAY][DRAW_OFFER_IN] = p_offdraw;
    packet_handlers[PLAY][RESIGN_IN] = p_resign;
    packet_handlers[PLAY][MESSAGE_IN] = p_message;
    registered = true;
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

void free_packet(struct packet* pc) {
    if (pc) {
        free(pc->data);
        free(pc);
    }
}

int send_packet(struct player* pl, struct packet* pc) {
    int ret;
    char* s; // the data we send
    unsigned long len; // the length of the data
    if (!pl || !pc) {
        return -1;
    }
    if (pl->fd < 0) {
        printf("Failed to send packet. Reason: player %s is disconnected\n", pl->name);
        return -2;
    }

    s = malloc(PACKET_HEADER_SIZE + strlen(pc->data) + 1);
    sprintf(s, PACKET_HEADER_FORMAT, PACKET_MAGIC_HEADER, pc->id, pc->len, pc->data);
    len = strlen(s);
    ret = send_raw(pl, s, &len);
    free(s);
    free_packet(pc);
    return ret;
}

struct packet* create_packet(unsigned int id, unsigned int len, const char* data) {
    struct packet* p = malloc(sizeof(struct packet));
    if (p == NULL) {
        return NULL;
    }
    p->id = id;
    p->len = len;
    p->data = malloc(sizeof(char) * (strlen(data) + 1));
    strcpy(p->data, data);
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
    handle = packet_handlers[pstate][id];
    if (!handle) {
        *erc = PACKET_ERR_INVALID_ID;
        return NULL;
    }
    *erc = 0;
    return handle;
}