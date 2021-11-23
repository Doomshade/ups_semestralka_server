#include <stdio.h>
#include <string.h>
#include "../include/packet_handler.h"

int handle_packet(struct player* pl, unsigned int id, char* data) {
    packet_handle* handle_func;
    int erc;

    if (!pl) {
        return -1;
    }

    printf("Handling packet - ID: %d, size: %lu, data: %s\n", id, strlen(data), data);

    if (pl->fd < 0) {
        printf("Player %s is currently disconnected, could not send the packet\n", pl->name);
        return -1;
    }
    handle_func = get_handler(id, pl->ps, &erc);

    // the handle func exists and the error code is 0 -> call and
    // return the func
    if (handle_func != NULL && !erc) {
        return handle_func(pl, data);
    }

    return erc;
}
