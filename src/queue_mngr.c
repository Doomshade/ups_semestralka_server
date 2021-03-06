#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "../include/queue_mngr.h"
#include "../include/packet_registry.h"

#define NOT_IN_QUEUE 0
#define IS_IN_QUEUE 1
#define VALIDATE_QUEUE(p) if(!p || !q || p->fd >= q->queue_len) return 1;

struct queue {
    int* queue;
    char** queue_key;
    unsigned queue_len;
};

static struct queue* q = NULL;

void init_qman(unsigned queue_len) {
    if (q) {
        return;
    }

    printf("Initializing queue manager...\n");
    q = malloc(sizeof(struct queue));
    q->queue_key = calloc(queue_len, sizeof(char*));
    q->queue = calloc(queue_len, sizeof(int));
    q->queue_len = queue_len;
}

int send_queue_out_pc(struct player* p, bool white, char* op) {
    char* buf;
    size_t siz;
    int ret;

    if (!p || !op) {
        return 1;
    }

    // +2 because of the "white" tag at the start
    siz = sizeof(char) * (strlen(START_FEN) + 2);
    buf = malloc(siz);
    memset(buf, 0, siz);
    strcpy(buf, white ? "1" : "0");
    strcat(buf, START_FEN);

    ret = send_packet(p, GAME_START_OUT, buf);
    free(buf);
    if (ret) {
        return ret;
    }
    ret = send_packet(p, OPPONENT_NAME_OUT, op);
    // TODO
    return ret;
}

/**
 * Finds the player in the queue. If the player was not in a queue before,
 * adds him to the queueKey array under the first free index and returns it
 * @param p the player
 * @return the index in queueKey and queue
 */
static int find_player_in_queue(struct player* p) {
    int i;
    int firstFree = -1;

    for (i = 0; i < q->queue_len; ++i) {
        // no player on this key
        if (!q->queue_key[i]) {
            // store the first free index in case the player is not found
            if (firstFree == -1) {
                firstFree = i;
            }
            continue;
        }

        // found a player under this ID
        if (strcmp(p->name, q->queue_key[i]) == 0) {
            return i;
        }
    }
    // the player was not in the queue, add its name
    q->queue_key[firstFree] = malloc(strlen(p->name) + 1);
    strcpy(q->queue_key[firstFree], p->name);

    return firstFree;
}

int add_to_queue(struct player* p) {
    int i;
    int q_state; // the state of the player in queue
    int ret;
    int id;
    struct player* op; // opponent

    VALIDATE_QUEUE(p)

    // first we find the player in the queueKey
    for (i = 0; i < q->queue_len; ++i) {
        if (!q->queue_key[i]) {
            continue;
        }
    }
    id = find_player_in_queue(p);
    q_state = q->queue[id];

    if (send_packet(p, QUEUE_OUT, RESPONSE_VALID)) {
        return 1;
    }
    switch (q_state) {
        case NOT_IN_QUEUE:
            printf("Adding %s to the queue...\n", p->name);

            q->queue[id] = IS_IN_QUEUE;
            change_state(p, QUEUE);

            // look for an opponent now
            for (i = 0; i < q->queue_len; ++i) {
                // skip the player
                if (i == id) {
                    continue;
                }

                // TODO perhaps do a better matchmaking, but for UPS this is enough ig
                if (q->queue[i] != IS_IN_QUEUE) {
                    continue;
                }
                // found a match!! get the opponent, remove them from the queue, and create a game

                // lookup the opponent

                // the player has disconnected, set the state to NOT_IN_QUEUE, so he doesn't get checked again
                if (lookup_player_by_name(q->queue_key[i], &op) || !op) {
                    free(q->queue_key[i]);
                    q->queue_key[i] = NULL;
                    q->queue[i] = NOT_IN_QUEUE;
                    continue;
                }


                // remove both players from the queue
                q->queue[id] = NOT_IN_QUEUE;
                q->queue[i] = NOT_IN_QUEUE;

                // yes this is perhaps utterly retarded, but basically
                // if the first packet fails, we don't even send the
                // second one and immediately remove both from queue
                if ((ret = send_queue_out_pc(p, true, op->name)) ||
                    (ret = send_queue_out_pc(op, false, p->name)));


                // create a new game
                if (!ret) {
                    printf("Creating a new game...\n");
                    ret = game_create(p, op);

                    // could not create a game
                    if (ret) {
                        printf("Failed to create a new game! (%d)\n", ret);
                    }
                }
                return ret;
            }
            return 0;
        case IS_IN_QUEUE:
            printf("%s is already in the queue\n", p->name);
            return -2;
        default:
            printf("Invalid queue state %d for fd %s\n", q_state, p->name);
            return -3;
    }
}

int qman_handle_dc(struct player* p) {
    if (!p) {
        return 1;
    }
    VALIDATE_FD(p->fd, 1, q->queue_len)
    return remove_from_queue(p);
}


int remove_from_queue(struct player* p) {
    int q_state;
    int ret;
    int id;

    VALIDATE_QUEUE(p)

    id = find_player_in_queue(p);
    q_state = q->queue[id];

    switch (q_state) {
        case NOT_IN_QUEUE:
            return 0;
        case IS_IN_QUEUE:
            printf("Removing %s from the queue\n", p->name);
            q->queue[id] = NOT_IN_QUEUE;
            if ((ret = send_packet(p, LEAVE_QUEUE_OUT, RESPONSE_VALID))) {
                return ret;
            }
            change_state(p, LOGGED_IN);
            return 0;
        default:
            printf("Invalid queue state %d for %s\n", q_state, p->name);
            return 1;
    }
}