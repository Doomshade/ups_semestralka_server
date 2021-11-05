#include <stdlib.h>
#include <stdio.h>
#include "../include/packet_handler.h"

// TODO char* -> packet*
int handle_packet(char* buf) {
    // CHESS01004AHOJ

    char ss[BUFSIZ];

    struct packet* p = create_packet(0, 0, buf);
    if (!p) {
        return -1;
    }

    return PACKET_OK;
}
