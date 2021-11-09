#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/packet_handler.h"
#include "../include/packet_in.h"

int handle_packet(int fd, struct packet* packet) {
    // CHESS01004AHOJ
    packet_handle* handle_func;
    struct player *p;
    int ret;

    if (packet == NULL) {
        return -1;
    }

    ret = lookup_player()
    p = create_player(fd, "");
    printf("ID: %d, size: %d, data: %s\n", packet->id, packet->len, packet->data);
    handle_func = get_handler(packet->id);
    if (handle_func == NULL) {
        return 1;
    }

    handle_func(p, packet->data);

    return 0;
}
