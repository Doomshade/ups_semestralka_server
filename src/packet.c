#include <stdlib.h>
#include <stdio.h>
#include "../include/packet.h"
#include "../include/packet_validator.h"
#include "../include/queue_mngr.h"

int p_hello(struct player* p, char* data) {
    int ret;
    char* buf; // the packet data we send
    const char* HI = "Hi %s!\n";
    struct packet* pckt;

    if (!p || !data) {
        return 1;
    }

    // handle the packet
    printf("FD: %d\n", p->fd);
    printf("Name (before): %s\n", p->name);
    p->name = data;
    printf("Name (after): %s\n", p->name);
    p->ps = LOGGED_IN;

    // the OUT packet data
    buf = malloc(sizeof(char) * (strlen("Hi !\n") + strlen(p->name) + 1));
    sprintf(buf, HI, p->name);

    // send the packet
    pckt = create_packet(HELLO_OUT, strlen(buf), buf);
    ret = send_packet(p, pckt);

    free(buf);
    free(pckt);
    // ret |= handle_possible_reconnection(&p);
    return ret;
}

int p_queue(struct player* p, char* data) {
    if (!p || !data) {
        return 1;
    }

    // check for the state and do things based on that
    switch (p->ps) {
        case LOGGED_IN: // the player wants to join the queue
            return add_to_queue(p->fd);
        case QUEUE: // the player wants to leave the queue
            return remove_from_queue(p->fd);
        default:
            return 1;
    }
}

int p_movepc(struct player* p, char* data) {

    return 0;
}

int p_offdraw(struct player* p, char* data) {
    return 0;
}

int p_resign(struct player* p, char* data) {
    return 0;
}
