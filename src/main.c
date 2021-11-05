#include <stdlib.h>
#include <stdio.h>
#include "../include/packet_handler.h"

int main(int argc, char* argv[]) {
    printf(PACKET_FORMAT, PACKET_MAGIC_HEADER, 5, 4);
    return EXIT_SUCCESS;
}