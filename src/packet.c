#include <stdlib.h>
#include <stdio.h>
#include "../include/packet.h"
#include "../include/packet_validator.h"
#include "../include/queue_mngr.h"

#define VALIDATE_PARAM(p) if (!p) return 1;
#define VALIDATE_PARAMS(p, data) VALIDATE_PARAM(p) VALIDATE_PARAM(data)

int p_hello(struct player* p, char* data) {
    int ret;
    char* buf; // the packet data we send
    const char* HI = "Hi %s!\n";
    struct packet* pckt;

    VALIDATE_PARAMS(p, data)

    // handle the packet
    p->name = data;
    //p->ps = LOGGED_IN;
    change_state(p, LOGGED_IN);

    // the OUT packet data
    buf = malloc(sizeof(char) * (strlen("Hi !\n") + strlen(p->name) + 1));
    sprintf(buf, HI, p->name);

    // send the packet
    pckt = create_packet(HELLO_OUT, strlen(buf), buf);
    ret = send_packet(p, pckt);

    free(buf);
    free(pckt);
    // ret |= handle_possible_reconnection(&p);
    return ret;
}

int p_queue(struct player* p, char* data) {
    VALIDATE_PARAM(p)

    // check for the state and do things based on that
    switch (p->ps) {
        case LOGGED_IN: // the player wants to join the queue
            return add_to_queue(p->fd);
        case QUEUE: // the player wants to leave the queue
            return remove_from_queue(p->fd);
        default:
            return 1;
    }
}

int p_movepc(struct player* p, char* data) {
    unsigned int rank_from; // A-H -> 0-7
    unsigned int rank_to; // A-H -> 0-7
    unsigned int file_from; // 1-8 -> 0-7
    unsigned int file_to; // 1-8 -> 0-7
    unsigned int max_bound; // 0-7
    struct game* g;
    struct packet* pc;
    char* msg;

    VALIDATE_PARAMS(p, data)

    g = lookup_game(p);
    if (!g) {
        return 1;
    }

    // the packet should be in "A2A4" format
    if (strlen(data) != 4) {
        return 1;
    }
    file_from = data[0] - 'A';
    file_to = data[2] - 'A';
    rank_from = data[1] - '1';
    rank_to = data[3] - '1';

    // basically wee should get a value 0-7 like that
    max_bound = rank_from | rank_to | file_from | file_to;
    if (max_bound >= 8) {
        msg = "INVALID";
        pc = create_packet(MOVE_OUT, strlen(msg), msg);
        if (!pc) {
            return 1;
        }

        send_packet(p, pc);
        return 1;
    }

    // check whose turn it is


    if (move_piece(g, p, rank_from, file_from, rank_to, file_to)) {
        printf("Invalid move!\n");
        return 1;
    }
    // TODO validate the move
    pc = create_packet(MOVE_OUT, strlen(data), data);
    send_packet(g->white, pc);
    send_packet(g->black, pc);

    return 0;
}

int p_offdraw(struct player* p, char* data) {
    struct game* g;
    char* PAYLOAD = "";

    VALIDATE_PARAM(p)

    // TODO add a timeout for the draw offer
    g = lookup_game(p);
    if (!g) {
        return 1;
    }

    return send_packet(g->white == p ? g->black : g->white,
                       create_packet(DRAW_OFFER_OUT, strlen(PAYLOAD), PAYLOAD));
}

int p_resign(struct player* p, char* data) {
    struct game* g;
    int winner;

    VALIDATE_PARAM(p)

    g = lookup_game(p);
    if (!g) {
        return 1;
    }
    if (g->white == p) {
        winner = BLACK_WINNER;
    } else {
        winner = WHITE_WINNER;
    }

    winner |= WIN_BY_RESIGNATION;
    return finish_game(g, winner);
}
