#include <stdlib.h>
#include "../include/packet.h"

struct packet* create_packet(unsigned int id, unsigned int len, char* data) {
    struct packet* p = malloc(sizeof(struct packet));
    if (!p) {
        return NULL;
    }
    p->id = id;
    p->len = len;
    p->data = data;
    return p;
}
