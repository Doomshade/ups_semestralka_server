#include <stdlib.h>
#include <stdio.h>
#include "../include/packet_validator.h"
#include "../include/server.h"
#include "../include/player_mngr.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

// for each fd make a buffer where the packets are sent
char** buffered_pheaders = NULL;
char** buffered_pdata = NULL;

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

void init_pvalidator() {

    buffered_pheaders = calloc(MAX_PLAYER_COUNT, sizeof(char) * (PACKET_HEADER_SIZE + 1));
    buffered_pdata = calloc(MAX_PLAYER_COUNT, sizeof(char*));
}

struct packet* parse_packet(char** packet, int* erc, struct player* pl) {
    // pl's fd
    int fd;

    struct packet* p;
    unsigned int p_id = 0; // packet id
    unsigned int p_data_len = 0; // packet data length
    char* pheader = NULL; // packet header
    char* pdata = NULL; // packet data
    char* p_data_copy = NULL; // packet data copy for packet struct

    // buffer header amount
    unsigned int buf_hamnt = 0;
    // buffer data amount
    unsigned int buf_damnt = 0;
    // length of the whole packet
    unsigned int p_len = 0;

    // return value
    int ret = 0;

    // validate params
    if (!erc) {
        return NULL;
    }
    if (!packet || !*packet || !pl) {
        *erc = PACKET_INVALID_DATA;
        return NULL;
    }

    // the buffered packet data not yet instantiated
    if (!buffered_pdata) {
        *erc = -2;
        return NULL;
    }
    *erc = PACKET_OK;
    fd = pl->fd;

    // concatenate it to the buffer
    // the header size should be either the
    // "header - curr_header_size" or the whole packet
    p_len = strlen(*packet);

    pheader = buffered_pheaders[fd];
    if (!pheader) {
        pheader = malloc(sizeof(char) * (PACKET_HEADER_SIZE + 1));
        memset(pheader, 0, sizeof(char) * (PACKET_HEADER_SIZE + 1));
        buffered_pheaders[fd] = pheader;
        // memset(p_header, 0, sizeof(char) * (PACKET_HEADER_SIZE + 1));
    }
    buf_hamnt = MIN(PACKET_HEADER_SIZE - strlen(pheader), p_len);
    strncat(pheader, *packet, buf_hamnt);

    // shift the packet by the header size (or 0 if the buffer is already full)
    *packet += buf_hamnt;

    // the p_header buffer is not yet full
    if (strlen(pheader) < PACKET_HEADER_SIZE) {
        printf("Strlen: %lu\n", strlen(pheader));
        printf("Pheader len: %lu\n", PACKET_HEADER_SIZE);
        printf("Header '%s' not yet fully buffered, %lu bytes remaining...\n", pheader,
               PACKET_HEADER_SIZE - strlen(pheader));
        *erc = PACKET_NOT_YET_FULLY_BUFFERED;
        return NULL;
    }

    // the header is invalid
    ret = validate_header(pheader, &p_id, &p_data_len);
    if (ret) {
        printf("Failed to parse header: %s\n", pheader);
        *erc = ret;
        return NULL;
    }

    // the p_header buffer is full and valid, concatenate to the data buffer now
    // the packet data is empty, initialize it to the packet length
    pdata = buffered_pdata[fd];
    if (!pdata) {
        pdata = malloc(sizeof(char) * (p_data_len + 1));
        memset(pdata, 0, sizeof(char) * (p_data_len + 1));
        buffered_pdata[fd] = pdata;
    }

    // the data amount should either be the
    // "size - curr_data_size" or the whole packet
    buf_damnt = MIN(p_data_len - strlen(pdata), p_len);
    strncat(pdata, *packet, buf_damnt);

    // shift the packet by the data
    *packet += buf_damnt;
    // the data is not yet full
    if (strlen(pdata) < p_data_len) {
        printf("Strlen: %lu\n", strlen(pdata));
        printf("Pdata len: %u\n", p_data_len);
        printf("Data %s not yet fully buffered, %lu bytes remaining...\n", pdata, p_data_len - strlen(pdata));
        *erc = PACKET_NOT_YET_FULLY_BUFFERED;
        return NULL;
    }
    // we now have the packet, the leftover data should be handled again
    // in a while loopstrcpy(pdata, "");
    p_data_copy = malloc(sizeof(char) * (p_data_len + 1));
    strcpy(p_data_copy, pdata);
    p = create_packet(p_id, p_data_len, p_data_copy);

    // free the buffers
    free_buffers(fd);

    return p;
}

void free_buffers(int fd) {
    if (buffered_pheaders[fd]) {
        free(buffered_pheaders[fd]);
        buffered_pheaders[fd] = NULL;
    }
    if (buffered_pdata[fd]) {
        free(buffered_pdata[fd]);
        buffered_pdata[fd] = NULL;
    }
}
