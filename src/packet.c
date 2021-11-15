#include <stdlib.h>
#include <stdio.h>
#include "../include/packet.h"
#include "../include/packet_validator.h"
#include "../include/queue_mngr.h"
#include "../include//server.h"

#define VALIDATE_PARAM(p) if (!p) return 1;
#define VALIDATE_PARAMS(p, data) VALIDATE_PARAM(p) VALIDATE_PARAM(data)

#ifdef __DEBUG_MODE
#define RESPONSE_VALID "Hi %s!"
#define RESPONSE_INVALID "Name exists!"
#else
#define RESPONSE_VALID "0"
#define RESPONSE_INVALID "1"
#endif


int p_hello(struct player* pl, char* data) {
    struct game* g;
    int ret;
#ifdef __DEBUG_MODE
    char* buf; // the packet data we send
#endif
    struct packet* pc;
    struct player* e_pl = NULL; // existing player

    VALIDATE_PARAMS(pl, data)

    ret = lookup_player_by_name(data, &e_pl);
    // the player exists
    if (!ret) {
        buf = malloc(sizeof(char) * (strlen(RESPONSE_INVALID)));
        strcpy(buf, RESPONSE_INVALID);
        pc = create_packet(HELLO_OUT, strlen(buf), buf);
        if (e_pl) {
            printf("A player with the name %s (%d) already exists!\n", e_pl->name, e_pl->fd);
        } else {
            printf("A player with name %s already exists!\n", data);
        }
        if (!pc) {
            printf("Failed to create a packet!\n");
            return 1;
        }
        ret = send_packet(pl, pc);
        if (ret) {
            printf("Failed to send packet!\n");
        }
        return 0;
    }

    // handle the packet
    ret = change_player_name(pl, data);
    if (ret) {
        printf("Player name too long! Max size is %d\n", MAX_PLAYER_NAME_LENGTH);
        return 0;
    }

    // the OUT packet data
#ifdef __DEBUG_MODE
    buf = malloc(sizeof(char) * (strlen(RESPONSE_VALID) + strlen(pl->name) + 1));
    sprintf(buf, RESPONSE_VALID, pl->name);
    pc = create_packet(HELLO_OUT, strlen(buf), buf);
#else
    pc = create_packet(HELLO_OUT, strlen(RESPONSE_VALID), RESPONSE_VALID);
#endif
    ret = send_packet(pl, pc);

#ifdef __DEBUG_MODE
    free(buf);
#endif
    if (ret) {
        printf("Failed to send the packet!\n");
        return 1;
    }

    ret = lookup_dc_player(pl->name, &pl);

    // the player previously disconnected
    if (!ret) {
        switch (pl->ps) {
            case PLAY: // the play was in-game, inform the players
                g = lookup_game(pl);
                if (!g) {
                    printf("The player %s was in state PLAY, but the game was not found!\n", pl->name);
                    change_state(pl, LOGGED_IN);
                    return 0;
                }
                return reconnect_to_game(pl, g);
            case QUEUE:
                change_state(pl, LOGGED_IN);
                return 0;
            default:
                break;
        }
    }
    change_state(pl, LOGGED_IN);

    // ret |= handle_possible_reconnection(&pl);
    return 0;
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
        printf("Incorrect format received\n");
        msg = "INVALID";
        pc = create_packet(MOVE_OUT, strlen(msg), msg);
        if (!pc) {
            return 1;
        }

        send_packet(p, pc);
        return 1;
    }
    file_from = data[0] - 'A';
    file_to = data[2] - 'A';
    rank_from = data[1] - '1';
    rank_to = data[3] - '1';

    // basically wee should get a value 0-7
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
        return 0; // perhaps the client miscalculated, don't dc the player
    }
    // TODO validate the move
    pc = create_packet(MOVE_OUT, strlen(data), data);
    send_packet(g->white, pc);

    pc = create_packet(MOVE_OUT, strlen(data), data);
    send_packet(g->black, pc);

    return 0;
}

int p_offdraw(struct player* p, char* data) {
    struct game* g;
    char* PAYLOAD = "";
    struct packet* pc;
    int ret;

    VALIDATE_PARAM(p)

    // TODO add a timeout for the draw offer
    g = lookup_game(p);
    if (!g) {
        return 1;
    }

    pc = create_packet(DRAW_OFFER_OUT, strlen(PAYLOAD), PAYLOAD);
    ret = send_packet(g->white == p ? g->black : g->white,
                      pc);
    return ret;
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

int p_message(struct player* p, char* packet) {
    struct game* g;
    struct player* op;
    struct packet* pc;
    char buf[BUFSIZ];

    VALIDATE_PARAMS(p, packet)

    sprintf(buf, "%.*s", MAX_MESSAGE_SIZE, packet);
    printf("[%s]: %s", p->name, buf);

    if (strlen(packet) > MAX_MESSAGE_SIZE) {
        printf(" (...)");
    }
    printf("\n");

    g = lookup_game(p);
    if (!g) {
        return 1;
    }
    op = OPPONENT(g, p);
    pc = create_packet(MESSAGE_OUT, strlen(buf), buf);
    send_packet(op, pc);
    return 0;
}
