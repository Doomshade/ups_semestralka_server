#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../include/packet.h"
#include "../include/packet_registry.h"
#include "../include/packet_validator.h"

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

int send_packet(struct player* pl, struct packet* pc) {
    int ret;
    char* s = malloc(PACKET_HEADER_SIZE + strlen(pc->data) + 1);
    unsigned long len;

    sprintf(s, PACKET_HEADER_FORMAT, PACKET_MAGIC_HEADER, pc->id, pc->len, pc->data);
    len = strlen(s);
    ret = send_raw(pl, s, &len);
    free(s);
    //sscanf(s, PACKET_MAGIC_HEADER, pc->id, pc->len, pc->data);
    return ret;
}

int p_hello(struct player* p, char* data) {
    int ret;
    char* buf;
    const char* HI = "Hi %s!\n";
    struct packet* pckt;
    if (p->ps != JUST_CONNECTED) {
        return CLIENT_SENT_PACKET_IN_INVALID_STATE;
    }
    printf("FD: %d\n", p->fd);
    printf("Name (before): %s\n", p->name);
    p->name = data;
    printf("Name (after): %s\n", p->name);
    p->ps = LOGGED_IN;

    buf = malloc(sizeof(char) * (strlen("Hi !\n") + strlen(p->name) + 1));
    sprintf(buf, HI, p->name);
    pckt = create_packet(HELLO_OUT, strlen(buf), buf);
    ret = send_packet(p, pckt);
    free(buf);
    free(pckt);
    // ret |= handle_possible_reconnection(&p);
    return ret;
}

int p_findgm(int fd, char* data) {
    return 0;
}

int p_movepc(int fd, char* data) {
    return 0;
}

int p_offdraw(int fd, char* data) {
    return 0;
}

int p_resign(int fd, char* data) {
    return 0;
}
