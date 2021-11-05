#include <sys/socket.h>
#include "../include/server.h"
#include "../include/game_mngr.h"

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

void startup() {

}