#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/packet_handler.h"
#include "../include/packet.h"

int handle_packet(struct player* pl, struct packet* pckt) {
    // CHESS01004AHOJ
    packet_handle* handle_func;
    struct player* p;

    if (pckt == NULL) {
        return -1;
    }

    printf("ID: %d, size: %d, data: %s\n", pckt->id, pckt->len, pckt->data);
    handle_func = get_handler(pckt->id);
    if (handle_func == NULL) {
        return 1;
    }

    return handle_func(pl, pckt->data);
}
