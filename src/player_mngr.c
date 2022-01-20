#include <string.h>
#include <stdio.h>
#include "../include/player_mngr.h"
#include "../include/server.h"
#include "../include/game_mngr.h"
#include "../include/packet_handler.h"
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_DISCONNECTED_COUNT MAX_PLAYER_COUNT * 5

#define KEEPALIVE_INTERVAL_RETRY 30
#define VALIDATE_FD(fd, ret) if (fd >= MAX_PLAYER_COUNT) return ret;

static struct player* players[MAX_PLAYER_COUNT] = {0};
static struct player* dc_players[MAX_DISCONNECTED_COUNT] = {0};

int lookup_player_by_fd(int fd, struct player** p) {
    struct player* _p;

    VALIDATE_FD(fd, 2)
    _p = players[fd];
    if (_p) {
        if (p) {
            *p = _p;
        }
        return 0;
    }
    return 1;
}

int lookup_player_by_name(char* name, struct player** p) {
    struct player* _p;
    int i;
    if (!name || !p) {
        return 1;
    }

    for (i = 0; i < MAX_PLAYER_COUNT; ++i) {
        if (!players[i]) {
            continue;
        }
        _p = players[i];
        if (strcmp(_p->name, name) == 0) {
            *p = _p;
            return 0;
        }
    }
    return 1;
}

int lookup_dc_player(char* name, struct player** p) {
    int i;
    struct player* _p;
    if (!name || !p) {
        return 1;
    }

    for (i = 0; i < MAX_DISCONNECTED_COUNT; ++i) {
        if (!dc_players[i]) {
            continue;
        }
        _p = dc_players[i];
        if (strcmp(_p->name, name) == 0) { // the player was previously disconnected
            printf("Found a disconnected player under index %d with name %s, FD %d, and state %d\n", i, _p->name,
                   _p->fd, _p->ps);
            _p->fd = (*p)->fd; // change the FD to the new one
            *p = _p; // return the previous state with the current FD
            players[_p->fd] = _p; // update the player in the array
            dc_players[i] = NULL; // remove the player from the dc_players
            return 0;
        }
    }
    return 1;
}

struct player* create_player(int fd) {
    struct player* p;

    VALIDATE_FD(fd, NULL)

    p = malloc(sizeof(struct player));
    if (!p) {
        return NULL;
    }
    memset(p, 0, sizeof(struct player));
    p->fd = fd;
    p->started_keepalive = 0;
    p->last_keepalive = 0;
    p->invalid_sends = 0;
    change_player_name(p, "New Player");
    change_state(p, JUST_CONNECTED);
    return p;
}

struct thr_args {
    struct player* p;
    unsigned keepalive_retry;
};

void* keepalive(void* _p) {
    time_t t;
    struct thr_args* p_args = (struct thr_args*) _p;
    struct player* p = p_args->p;
    unsigned interval = p_args->keepalive_retry;
    double diff;

    while (1) {
        // update the current time
        time(&t);

        // get the difference in seconds between the last keep alive packets
        diff = difftime(t, p->last_keepalive);

        // set the last time to the current time
        p->last_keepalive = t;

        // if everything is okay, we only sleep
        if (diff < interval) {
            // sleep for one second before checking again
            // reduces overhead
            sleep(1);
            continue;
        }

        // TODO dc the player
        handle_dc(p);
        pthread_exit(NULL);
        return NULL;


    }
    return NULL;
}

void start_keepalive(struct player* p, unsigned keepalive_retry) {
    struct thr_args args;
    pthread_t thread;

    if (p->started_keepalive) {
        return;
    }
    args.p = p;
    args.keepalive_retry = keepalive_retry;

    printf("Starting keepalive for %s...\n", p->name);
    p->started_keepalive = 1;
    pthread_create(&thread, NULL, keepalive, &args);
}

int change_player_name(struct player* p, char* name) {
    if (!p || !name) {
        return 2;
    }
    if (strlen(name) >= MAX_PLAYER_NAME_LENGTH) {
        return 1;
    }
    strcpy(p->name, name);
    return 0;
}

int handle_new_player(int fd) {
    VALIDATE_FD(fd, 1)

    // the player under this FD is already connected
    // this should not happen, if this
    // happens some logic inside the
    // server failed
    if (players[fd]) {
        return 2;
    }

    // add the client to the connected players
    players[fd] = create_player(fd);
    return players[fd] != NULL;
}

void change_state(struct player* p, enum player_state ps) {
    char* ps_str;
    p->ps = ps;
    switch (p->ps) {
        case JUST_CONNECTED:
            ps_str = "connected";
            break;
        case LOGGED_IN:
            ps_str = "logged in";
            break;
        case QUEUE:
            ps_str = "queue";
            break;
        case PLAY:
            ps_str = "play";
            break;
        default:
            ps_str = "unknown";
            break;
    }
    printf("Changed %s's state to: %s\n", p->name, ps_str);
}

int pman_handle_dc(struct player* p) {
    int i;

    if (!p) {
        return 1;
    }
    VALIDATE_FD(p->fd, 2)

    players[p->fd] = NULL;

#ifdef SERVER_DEBUG_MODE
    if (!lookup_player_by_fd(p->fd, NULL)) {
        printf("Player is still in memory somehow...\n");
    }
#endif

    // add him to the disconnected list
    // CAUTION: do not store it under the
    // clients FD! a situation like:
    // a client connects with an FD of 4,
    // joins a game, disconnects,
    // a different client joins and is given
    // the FD 4 and that client would be reconnected!|
    for (i = 0; i < MAX_DISCONNECTED_COUNT; ++i) {
        if (dc_players[i] == NULL) {
            printf("Storing %s under %d in the disconnected list\n", p->name, i);
            p->fd = -1;
            p->started_keepalive = 0;
            dc_players[i] = p;
            return 0;
        }
    }

    return 1;
}