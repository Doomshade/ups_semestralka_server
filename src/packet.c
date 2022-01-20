#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../include/packet.h"
#include "../include/packet_validator.h"
#include "../include/queue_mngr.h"

#define VALIDATE_PARAM(p) if (!p) return 1;
#define VALIDATE_PARAMS(p, data) VALIDATE_PARAM(p) VALIDATE_PARAM(data)

int p_hello(struct player* pl, char* data) {
    struct game* g;
    int ret;
    struct player* e_pl = NULL; // existing player
    const int keepalive_retry = 30; // TODO make this a var

    VALIDATE_PARAMS(pl, data)

    ret = lookup_player_by_name(data, &e_pl);
    // the player exists
    if (!ret) {
        if (e_pl) {
            printf("A player with the name %s (%d) already exists!\n", e_pl->name, e_pl->fd);
        } else {
            printf("A player with name %s already exists!\n", data);
        }
        ret = send_packet(pl, HELLO_OUT, RESPONSE_INVALID);
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

    // the name is valid
    ret = send_packet(pl, HELLO_OUT, RESPONSE_VALID);

    if (ret) {
        printf("Failed to send the packet!\n");
        return 1;
    }
    //start_keepalive(pl, keepalive_retry);

    ret = lookup_dc_player(pl->name, &pl);
    // the player previously disconnected
    if (!ret) {
        switch (pl->ps) {
            case PLAY: // the player was in-game, inform the players
                g = lookup_game(pl);
                if (!g) {
                    printf("The player %s was in state PLAY, but the game was not found!\n", pl->name);
                    change_state(pl, LOGGED_IN);
                    return 0;
                }
                return reconnect_to_game(pl, g);
            case QUEUE: // the player was in queue, put him back to queue
                return add_to_queue(pl);
            default:
                break;
        }
    }
    change_state(pl, LOGGED_IN);

    // ret |= handle_possible_reconnection(&pl);
    return 0;
}

int p_leave_queue(struct player* p, char* data) {
    VALIDATE_PARAM(p)

    return remove_from_queue(p);
}

int p_queue(struct player* p, char* data) {
    VALIDATE_PARAM(p)
    return add_to_queue(p);
}

int p_movepc(struct player* p, char* data) {
    unsigned int rank_from; // 1-8 -> 0-7
    unsigned int rank_to; // 1-8 -> 0-7
    unsigned int file_from; // A-H -> 0-7
    unsigned int file_to; // A-H -> 0-7
    unsigned int max_bound; // 0-7
    struct square* from;
    struct square* to;
    struct move* m;
    unsigned int move_id;
    char move_id_buffer[4];
    char response_valid[3 + strlen(RESPONSE_VALID) + 1];
    char response_invalid[3 + strlen(RESPONSE_INVALID) + 1];
    char* ptr;
    struct game* g;
    const char* RESPONSE_FORMAT = "%03d%s";

    VALIDATE_PARAMS(p, data)

    g = lookup_game(p);
    if (!g) {
        return 1;
    }

    // the packet should be in "123A2A4" format
    if (strlen(data) != 7) {
        sprintf(response_invalid, RESPONSE_FORMAT, 0, RESPONSE_INVALID);
        printf("Incorrect format received\n");
        //send_packet(p, MOVE_RESPONSE, response_invalid);
        return 1;
    }
    // TODO check if the num is right
    strncpy(move_id_buffer, data, 3);
    move_id = strtoul(move_id_buffer, &ptr, 10);
    sprintf(response_invalid, RESPONSE_FORMAT, move_id, RESPONSE_INVALID);

    file_from = FILE_TO_UINT(data[3]);
    file_to = FILE_TO_UINT(data[5]);
    rank_from = RANK_TO_UINT(data[4]);
    rank_to = RANK_TO_UINT(data[6]);

    // basically wee should get a value 0-7
    max_bound = rank_from | rank_to | file_from | file_to;
    if (max_bound >= 8) {
        //send_packet(p, MOVE_RESPONSE, response_invalid);
        return 1;
    }

    from = create_square(file_from, rank_from);
    to = create_square(file_to, rank_to);
    m = create_move(from, to);

    if (move_piece(g, p, m)) {
        printf("Invalid move!\n");
        free_move(&m);
        //send_packet(p, MOVE_RESPONSE, response_invalid);
        return 0; // perhaps the client miscalculated, don't dc the player
    }

    sprintf(response_valid, RESPONSE_FORMAT, move_id, RESPONSE_VALID);
    //send_packet(p, MOVE_RESPONSE, response_valid);
    free_move(&m);
    send_packet(p, MOVE_OUT, data + 3);
    send_packet(OPPONENT(g, p), MOVE_OUT, data + 3);

    return 0;
}

int p_offdraw(struct player* p, char* data) {
    struct game* g;
    char* PAYLOAD = "";
    int ret;

    VALIDATE_PARAM(p)

    // TODO add a timeout for the draw offer
    g = lookup_game(p);
    if (!g) {
        return 1;
    }

    ret = send_packet(g->white == p ? g->black : g->white, DRAW_OFFER_OUT, PAYLOAD);
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
    send_packet(op, MESSAGE_OUT, buf);
    return 0;
}

int p_keepalive(struct player* p, char* packet) {
    p->last_keepalive = time(NULL);
    return send_packet(p, KEEP_ALIVE_OUT, "");
}
