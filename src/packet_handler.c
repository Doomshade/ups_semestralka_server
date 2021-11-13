#include <stdio.h>
#include <string.h>
#include "../include/packet_handler.h"

int handle_packet(struct player* pl, struct packet* pckt) {
    // CHESS01004AHOJ
    packet_handle* handle_func;
    int erc;

    if (!pl || !pckt) {
        return -1;
    }

    printf("ID: %d, size: %d, data: %s\n", pckt->id, pckt->len, pckt->data);
    handle_func = get_handler(pckt->id, pl->ps, &erc);

    // the handle func exists and the error code is 0 -> call and
    // return the func
    if (handle_func != NULL && !erc) {
        return handle_func(pl, pckt->data);
    }

    return erc;
}
