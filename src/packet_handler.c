#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/packet_handler.h"
#include "../include/packet_in.h"

#define PACKET_MAGIC_HEADER "CHESS"
#define PACKET_FORMAT "%04s%02d%03d"

int handle_packet(int fd, struct packet* packet) {
    // CHESS01004AHOJ

    printf("ID: %d, size: %d, data: %s\n", packet->id, packet->len, packet->data);
    get_handler(packet->id)(fd, packet->data);

    return 0;
}
