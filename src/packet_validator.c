#include <stdlib.h>
#include <stdio.h>
#include "../include/packet_validator.h"

int validate_header(char* header, unsigned int* p_id, unsigned int* p_siz) {
    char* tmp;
    char s_id[PACKET_ID_LENGTH + 1];
    char s_siz[PACKET_SIZE_LENGTH + 1];
    char* end;
    int i;

    if (header == NULL || p_id == NULL || p_siz == NULL) {
        return -1;
    }
    if (strlen(header) != PACKET_HEADER_SIZE) {
        return PACKET_INVALID_HEADER_LENGTH;
    }
    if (strncmp(header, PACKET_MAGIC_HEADER, strlen(PACKET_MAGIC_HEADER)) != 0) {
        return PACKET_INVALID_HEADER_MAGIC;
    }

    // CHESS01004
    tmp = header;
    printf("Parsing %s\n", tmp);

    // 01004
    tmp += strlen(PACKET_MAGIC_HEADER);
    printf("Skipped to %s\n", tmp);
    for (i = 0; i < PACKET_ID_LENGTH; ++i, tmp++) {
        s_id[i] = *tmp;
    }
    s_id[PACKET_ID_LENGTH] = '\0';

    printf("Parsing ID: %s\n", s_id);
    *p_id = strtol(s_id, &end, 16);
    if (strcmp(end, "") != 0) {
        return PACKET_INVALID_HEADER_ID;
    }

    // 004
    for (i = 0; i < PACKET_SIZE_LENGTH; ++i, tmp++) {
        s_siz[i] = *tmp;
    }
    s_siz[PACKET_SIZE_LENGTH] = '\0';

    printf("Parsing size: %s\n", s_siz);
    *p_siz = strtol(s_siz, &end, 10);
    if (strcmp(end, "") != 0) {
        return PACKET_INVALID_HEADER_PACKET_SIZE;
    }
    return PACKET_OK;
}

struct packet* parse_packet(char* packet, int* erc) {
    unsigned int p_id;
    unsigned int p_siz;
    char header[PACKET_HEADER_SIZE + 1];

    *erc = PACKET_OK;
    if (strlen(packet) <= PACKET_HEADER_SIZE) {
        *erc = PACKET_INVALID_HEADER_LENGTH;
        return NULL;
    }

    strncpy(header, packet, PACKET_HEADER_SIZE);
    header[PACKET_HEADER_SIZE] = '\0';
    *erc = validate_header(header, &p_id, &p_siz);
    if (*erc != PACKET_OK) {
        return NULL;
    }

    if (strlen(packet) != PACKET_HEADER_SIZE + p_siz) {
        *erc = PACKET_INVALID_SIZE;
        return NULL;
    }
    struct packet* p = create_packet(p_id, p_siz, (packet + PACKET_HEADER_SIZE));
    return p;
}
