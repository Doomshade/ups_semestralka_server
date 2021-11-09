#include <stdlib.h>
#include <stdio.h>
#include "../include/packet_in.h"

int p_hello(int fd, char* data) {

    printf("FD: %d, name: %s\n", fd, data);
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
