#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../include/packet_in.h"

int send_packet(struct player* p, char* buf, long* len) {
    long total = 0; // how many bytes we've sent
    long bytesleft = *len; // how many we have left to send
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

int p_hello(struct player* p, char* data) {

    printf("FD: %d\n", p->fd);
    printf("Name (B): %s\n", p->name);
    p->name = data;
    printf("Name (A): %s\n", p->name);
    return 0;
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
