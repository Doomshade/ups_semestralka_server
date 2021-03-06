#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../include/packet.h"
#include "../include/packet_validator.h"
#include "../include/queue_mngr.h"
#include "../include/chesspiece.h"

#define VALIDATE_PARAM(p) if (!p) return 1;
#define VALIDATE_PARAMS(p, data) VALIDATE_PARAM(p) VALIDATE_PARAM(data)
#define MESSAGE_FORMAT "[%s]:\t%.*s"

int p_hello(struct player* pl, char* data) {
    struct game* g;
    int ret;
    struct player* e_pl = NULL; // existing player
    const int keepalive_retry = 60; // TODO make this a var

    VALIDATE_PARAMS(pl, data)

    ret = lookup_player_by_name(data, &e_pl);
    // the player exists
    if (!ret) {
        if (e_pl) {
            printf("A player with the name %s (%d) already exists!\n", e_pl->name, e_pl->fd);
        } else {
            printf("A player with name %s already exists!\n", data);
        }
        return !send_packet(pl, HELLO_OUT, RESPONSE_INVALID) ? PACKET_RESP_OK_INVALID_DATA : PACKET_RESP_ERR_NOT_RECVD;
    }

    // the name is too long
    ret = change_player_name(pl, data);
    if (ret) {
        printf("Player name too long! Max size is %d\n", MAX_PLAYER_NAME_LENGTH);
        return !send_packet(pl, HELLO_OUT, RESPONSE_INVALID) ? PACKET_RESP_OK_INVALID_DATA : PACKET_RESP_ERR_NOT_RECVD;
    }

    // the name is valid, send a resposne
    if (send_packet(pl, HELLO_OUT, RESPONSE_VALID)) {
        printf("Failed to send the packet!\n");
        return PACKET_RESP_ERR_NOT_RECVD;
    }

    ret = lookup_dc_player(pl->name, &pl);
    start_keepalive(pl->fd, keepalive_retry);
    // the player previously disconnected
    if (!ret) {
        switch (pl->ps) {
            case PLAY: // the player was in-game, inform the players
                g = lookup_game(pl);
                if (!g) {
                    printf("The player %s was in state PLAY, but the game was not found!\n", pl->name);
                    change_state(pl, LOGGED_IN);
                    return PACKET_RESP_OK;
                }
                return reconnect_to_game(pl, g) == 0 ? PACKET_RESP_OK : PACKET_RESP_OK_INVALID_DATA;
            case QUEUE: // the player was in queue, put him back to queue
                return add_to_queue(pl) == 0 ? PACKET_RESP_OK : PACKET_RESP_OK_INVALID_DATA;
            default:
                break;
        }
    }
    change_state(pl, LOGGED_IN);

    // ret |= handle_possible_reconnection(&pl);
    return PACKET_RESP_OK;
}

int p_leave_queue(struct player* p, char* data) {
    VALIDATE_PARAM(p)

    return !remove_from_queue(p) ? PACKET_RESP_OK : PACKET_RESP_ERR_INVALID_DATA;
}

int p_queue(struct player* p, char* data) {
    VALIDATE_PARAM(p)
    return !add_to_queue(p) ? PACKET_RESP_OK : PACKET_RESP_ERR_INVALID_DATA;
}

int p_movepc(struct player* p, char* data) {
    unsigned int rank_from; // 1-8 -> 0-7
    unsigned int rank_to; // 1-8 -> 0-7
    unsigned int file_from; // A-H -> 0-7
    unsigned int file_to; // A-H -> 0-7
    unsigned int max_bound; // 0-7
    int ret;
    struct square* from;
    struct square* to;
    struct move* m;
    unsigned int move_id;
    char buf[BUFSIZ];
    char move_id_buf[4];
    char response_valid_buf[3 + strlen(RESPONSE_VALID) + 1];
    char response_invalid_buf[3 + strlen(RESPONSE_INVALID) + 1];
    char* ptr;
    struct game* g;
    const char* RESPONSE_FORMAT = "%03d%s";

    VALIDATE_PARAMS(p, data)

    g = lookup_game(p);
    if (!g) {
        return PACKET_RESP_ERR_INVALID_DATA;
    }

    // the packet should be in "123A2A4" format
    if (strlen(data) != 7) {
        sprintf(response_invalid_buf, RESPONSE_FORMAT, 0, RESPONSE_INVALID);
        printf("Incorrect format received\n");
        //send_packet(p, MOVE_RESPONSE, response_invalid);
        return PACKET_RESP_ERR_INVALID_DATA;
    }
    // TODO check if the num is right
    strncpy(move_id_buf, data, 3);
    move_id = strtoul(move_id_buf, &ptr, 10);
    sprintf(response_invalid_buf, RESPONSE_FORMAT, move_id, RESPONSE_INVALID);

    file_from = FILE_TO_UINT(data[3]);
    file_to = FILE_TO_UINT(data[5]);
    rank_from = RANK_TO_UINT(data[4]);
    rank_to = RANK_TO_UINT(data[6]);

    // basically wee should get a value 0-7
    max_bound = rank_from | rank_to | file_from | file_to;
    if (max_bound >= 8) {
        //send_packet(p, MOVE_RESPONSE, response_invalid);
        return PACKET_RESP_ERR_INVALID_DATA;
    }

    from = create_square(file_from, rank_from);
    to = create_square(file_to, rank_to);
    m = create_move(from, to);

    ret = move_piece(g, p, m);
    if (ret == MOVE_INVALID) {
        printf("Invalid move!\n");
        free_move(&m);
        send_packet(p, MOVE_RESPONSE_OUT, response_invalid_buf);
        return PACKET_RESP_OK_INVALID_DATA; // perhaps the client miscalculated, don't dc the player
    }

    free_move(&m);
    sprintf(response_valid_buf, RESPONSE_FORMAT, move_id, RESPONSE_VALID);

    // the move is castles, send a special packet and that the move is valid
    if (ret & MOVE_CASTLES) {
        sprintf(buf, "%1d%1d", ret & MOVE_WHITE_CASTLES ? 1 : 0, ret & MOVE_LONG_CASTLES ? 1 : 0);
        if (send_packet(p, MOVE_RESPONSE_OUT, response_valid_buf) ||
            send_packet(p, MOVE_CASTLES_OUT, buf) ||
            send_packet(OPPONENT(g, p), MOVE_CASTLES_OUT, buf)) {
            return PACKET_RESP_ERR_NOT_RECVD;
        }
        return PACKET_RESP_OK;
    }

    if (ret == MOVE_EN_PASSANT) {
        sprintf(buf, "%1c%1c", UINT_TO_FILE(g->lm->file), UINT_TO_RANK(g->lm->rank));
        if (send_packet(p, MOVE_EN_PASSANT_OUT, buf) ||
            send_packet(OPPONENT(g, p), MOVE_EN_PASSANT_OUT, buf)) {
            return PACKET_RESP_ERR_NOT_RECVD;
        }
    }


    // the move is a regular move
    if (send_packet(p, MOVE_OUT, data + 3) ||
        send_packet(OPPONENT(g, p), MOVE_OUT, data + 3)) {
        return PACKET_RESP_ERR_NOT_RECVD;
    }

    if (send_packet(p, MOVE_RESPONSE_OUT, response_valid_buf)) {
        return PACKET_RESP_ERR_NOT_RECVD;
    }

    if (ret == MOVE_CHECKMATE) {
        if (finish_game(g, (g->white == p ? WHITE : BLACK) | WIN_BY_MATE)) {
            return PACKET_RESP_ERR_NOT_RECVD;
        }
    }


    return PACKET_RESP_OK;
}

int p_offdraw(struct player* p, char* data) {
    struct game* g;
    char* PAYLOAD = "";

    VALIDATE_PARAM(p)

    // TODO add a timeout for the draw offer
    g = lookup_game(p);
    if (!g) {
        return PACKET_RESP_ERR_INVALID_DATA;
    }

    return !send_packet(g->white == p ? g->black : g->white, DRAW_OFFER_OUT, PAYLOAD) ? PACKET_RESP_OK
                                                                                      : PACKET_RESP_ERR_NOT_RECVD;
}

int p_resign(struct player* p, char* data) {
    struct game* g;
    int winner;

    VALIDATE_PARAM(p)

    g = lookup_game(p);
    if (!g) {
        return PACKET_RESP_ERR_INVALID_DATA;
    }
    if (g->white == p) {
        winner = BLACK;
    } else {
        winner = WHITE;
    }

    winner |= WIN_BY_RESIGNATION;
    return !finish_game(g, winner) ? PACKET_RESP_OK : PACKET_RESP_ERR_NOT_RECVD;
}

int p_message(struct player* p, char* packet) {
    struct game* g;
    struct player* op;
    char buf[BUFSIZ];

    VALIDATE_PARAMS(p, packet)

    g = lookup_game(p);
    if (!g) {
        return PACKET_RESP_ERR_INVALID_DATA;
    }
    op = OPPONENT(g, p);

    sprintf(buf, MESSAGE_FORMAT, p->name, MAX_MESSAGE_SIZE, packet);
    printf("%s", buf);

    if (strlen(packet) > MAX_MESSAGE_SIZE) {
        printf(" (...)");
    }
    printf("\n");
    return !send_packet(op, MESSAGE_OUT, buf) ? PACKET_RESP_OK : PACKET_RESP_ERR_NOT_RECVD;
}

int p_keepalive(struct player* p, char* packet) {
    time_t t;
    time(&t);
    p->last_keepalive = t;
    return !send_packet(p, KEEP_ALIVE_OUT, "") ? PACKET_RESP_OK : PACKET_RESP_ERR_NOT_RECVD;
}
