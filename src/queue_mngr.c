#include <stdio.h>
#include "stdlib.h"
#include "../include/queue_mngr.h"
#include "../include/server.h"
#include "../include/player_mngr.h"

#define NOT_IN_QUEUE 0
#define IS_IN_QUEUE 1
#define VALIDATE_QUEUE(fd) if(!queue || fd >= MAX_PLAYER_COUNT) return 1;

// TODO make this an actual queue someday maybe?
int* queue = NULL;

void init_queue() {
    // the queue has been already initialized
    if (queue) {
        return;
    }
    printf("Initializing queue...\n");
    queue = calloc(MAX_PLAYER_COUNT, sizeof(int));
}

int add_to_queue(int fd) {
    int q_state; // the state of the player in queue
    int i;

    VALIDATE_QUEUE(fd)
    q_state = queue[fd];

    switch (q_state) {
        case NOT_IN_QUEUE:
            printf("Adding %d to the queue...\n", fd);
            queue[fd] = IS_IN_QUEUE;

            // check for a game now
            for (i = 0; i < MAX_PLAYER_COUNT; ++i) {
                // skip the player
                if (i == fd) {
                    continue;
                }

                // TODO perhaps do a better matchmaking, but for UPS this is enough ig
                // found a match
                if (queue[i] == IS_IN_QUEUE){

                    remove_from_queue(fd);
                    remove_from_queue(i);
                }
            }
            return 0;
        case IS_IN_QUEUE:
            printf("%d is already in the queue\n", fd);
            return 0;
        default:
            printf("Invalid queue state %d for fd %d\n", q_state, fd);
            return 1;
    }
}

int remove_from_queue(int fd) {
    int q_state;

    VALIDATE_QUEUE(fd)
    q_state = queue[fd];

    switch (q_state) {
        case NOT_IN_QUEUE:
            printf("%d is not in the queue", fd);
            return 0;
        case IS_IN_QUEUE:
            printf("%d has been removed from the queue\n", fd);
            queue[fd] = NOT_IN_QUEUE;
            return 0;
        default:
            printf("Invalid queue state %d for fd %d\n", q_state, fd);
            return 1;
    }
}