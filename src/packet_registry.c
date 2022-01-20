#include "../include/packet_registry.h"
#include "../include/packet.h"
#include "../include/packet_validator.h"
#include <stdlib.h>
#include <stdio.h>

// the amount of IN packets = (0 -- PACKET_OUT_OFFSET)
#define PACKET_COUNT PACKET_OUT_OFFSET
#define STATE_COUNT 4
// I don't calloc this because callocing matrices is hard :((
packet_handle* packet_handlers[STATE_COUNT][PACKET_COUNT] = {0};
bool registered = false;


void init_preg() {
    int i;
    if (registered) {
        return;
    }

    printf("Initializing packet registry...\n");
    packet_handlers[JUST_CONNECTED][HELLO_IN] = p_hello;
    packet_handlers[LOGGED_IN][QUEUE_IN] = p_queue;
    packet_handlers[QUEUE][LEAVE_QUEUE_IN] = p_leave_queue;
    packet_handlers[PLAY][MOVE_IN] = p_movepc;
    packet_handlers[PLAY][DRAW_OFFER_IN] = p_offdraw;
    packet_handlers[PLAY][RESIGN_IN] = p_resign;
    packet_handlers[PLAY][MESSAGE_IN] = p_message;

    // register keep alive packet in all but just connected states
    for (i = LOGGED_IN; i <= PLAY; ++i) {
        packet_handlers[i][KEEP_ALIVE_IN] = p_keepalive;
    }

    registered = true;
}

static int send_raw(struct player* p, char* buf, unsigned long* len) {
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
    return n == -1 ? PACKET_RESP_ERR_NOT_RECVD : PACKET_RESP_OK; // return -1 on failure, 0 on success
}

void free_packet(struct packet** pc) {
    if (pc && *pc) {
        free((*pc)->data);
        free(*pc);
        *pc == NULL;
    }
}

int send_packet(struct player* pl, unsigned int id, const char* data) {
    int ret;
    char* s; // the data we send
    unsigned long len; // the length of the data
    if (!pl) {
        return PACKET_RESP_ERR_NOT_RECVD;
    }
    if (pl->fd < 0) {
        printf("Failed to send packet. Reason: player %s is disconnected\n", pl->name);
        return PACKET_RESP_ERR_NOT_RECVD;
    }

    s = malloc(PACKET_HEADER_SIZE + strlen(data) + 1);
    sprintf(s, PACKET_HEADER_FORMAT, PACKET_MAGIC_HEADER, id, strlen(data), data);
    len = strlen(s);
    printf("Sending packet %s to %s\n", s, pl->name);
    ret = send_raw(pl, s, &len);
    free(s);
    return ret;
}

struct packet* create_packet(unsigned int id, const char* data) {
    struct packet* p = malloc(sizeof(struct packet));
    if (p == NULL) {
        return NULL;
    }
    p->id = id;
    p->len = strlen(data);
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