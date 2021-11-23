#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "../include/queue_mngr.h"
#include "../include/server.h"
#include "../include/packet_registry.h"

#define NOT_IN_QUEUE 0
#define IS_IN_QUEUE 1
#define VALIDATE_QUEUE(p) if(!queue || p->fd >= MAX_PLAYER_COUNT) return 1;

// TODO make this an actual queue someday maybe?
int* queue = NULL;

void init_qman() {
    if (queue) {
        return;
    }

    printf("Initializing queue manager...\n");
    queue = calloc(MAX_PLAYER_COUNT, sizeof(int));
}

int send_queue_out_pc(struct player* p, bool white, char* op) {
    char* buf;
    size_t siz;
    struct packet* pc;
    int ret;

    if (!p || !op) {
        return 1;
    }

    // +2 because of the "white" tag at the start
    siz = sizeof(char) * (strlen(op) + 2);
    buf = malloc(siz);
    memset(buf, 0, siz);
    strcpy(buf, white ? "0" : "1");
    strcat(buf, op);

    pc = create_packet(GAME_START_OUT, buf);
    if (!pc) {
        free(buf);
        return 1;
    }
    ret = send_packet(p, pc);
    if (ret) {
        free(buf);
        return ret;
    }
    free(buf);
    // TODO
    // pc = create_packet(OPPONENT_NAME_OUT, strlen());
    return ret;
}

int add_to_queue(struct player* p) {
    int i;
    int q_state; // the state of the player in queue
    int ret;
    int fd;
    struct packet* pc;
    struct player* op; // opponent

    VALIDATE_QUEUE(p)
    fd = p->fd;
    q_state = queue[fd];

    switch (q_state) {
        case NOT_IN_QUEUE:
            printf("Adding %s to the queue...\n", p->name);
            queue[fd] = IS_IN_QUEUE;

            pc = create_packet(QUEUE_OUT, "0");
            if (!pc) {
                return 1;
            }
            if (send_packet(p, pc)) {
                return 1;
            }
            change_state(p, QUEUE);

            // check for a game now
            for (i = 0; i < MAX_PLAYER_COUNT; ++i) {
                // skip the player
                if (i == fd) {
                    continue;
                }

                // TODO perhaps do a better matchmaking, but for UPS this is enough ig
                // found a match
                if (queue[i] == IS_IN_QUEUE) {
                    lookup_player_by_fd(i, &op);
                    if (!op) {
                        return 1;
                    }

                    // yes this is perhaps utterly retarded, but basically
                    // if the first packet fails, we don't even send the
                    // second one and immediately remove both from queue
                    if ((ret = send_queue_out_pc(p, true, op->name)) ||
                        (ret = send_queue_out_pc(op, false, p->name)));

                    remove_from_queue(p);
                    remove_from_queue(op);

                    if (!ret) {
                        printf("Creating a new game...\n");
                        ret = game_create(p, op);
                        if (ret) {
                            printf("Failed to create a new game! (%d)\n", ret);
                        }
                    }
                    return ret;
                }
            }
            return 0;
        case IS_IN_QUEUE:
            printf("%s is already in the queue\n", p->name);
            return 0;
        default:
            printf("Invalid queue state %d for fd %s\n", q_state, p->name);
            return 1;
    }
}

int remove_from_queue(struct player* p) {
    int q_state;
    struct packet* pc;
    int ret;
    int fd;

    VALIDATE_QUEUE(p)
    fd = p->fd;
    q_state = queue[fd];

    switch (q_state) {
        case NOT_IN_QUEUE:
            return 0;
        case IS_IN_QUEUE:
            printf("Removing %s from the queue\n", p->name);
            queue[fd] = NOT_IN_QUEUE;
            pc = create_packet(QUEUE_OUT, "1");
            if (!pc) {
                return 1;
            }

            if ((ret = send_packet(p, pc))) {
                return ret;
            }
            change_state(p, LOGGED_IN);
            return 0;
        default:
            printf("Invalid queue state %d for %s\n", q_state, p->name);
            return 1;
    }
}