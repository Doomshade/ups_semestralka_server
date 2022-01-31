#include <string.h>
#include <stdio.h>
#include "../include/player_mngr.h"
#include "../include/server.h"
#include "../include/game_mngr.h"
#include <time.h>
#include <pthread.h>
#include <unistd.h>

struct players {
    struct player** players;
    struct player** dc_players;
    unsigned max_player_count;
};

static struct players* plrs = NULL;

void init_pman(unsigned player_count) {
    if (plrs) {
        return;
    }

    plrs = malloc(sizeof(struct players));
    plrs->max_player_count = player_count;
    plrs->players = calloc(plrs->max_player_count, sizeof(struct player*));
    plrs->dc_players = calloc(plrs->max_player_count, sizeof(struct player*));
}


int lookup_player_by_fd(int fd, struct player** p) {
    struct player* _p;

    VALIDATE_FD(fd, 2, plrs->max_player_count)
    _p = plrs->players[fd];
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

    for (i = 0; i < plrs->max_player_count; ++i) {
        if (!plrs->players[i]) {
            continue;
        }
        _p = plrs->players[i];
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

    for (i = 0; i < plrs->max_player_count; ++i) {
        if (!plrs->dc_players[i]) {
            continue;
        }
        _p = plrs->dc_players[i];
        if (strcmp(_p->name, name) == 0) { // the player was previously disconnected
            printf("Found a disconnected player under index %d with name %s, FD %d, and state %d\n", i, _p->name,
                   _p->fd, _p->ps);
            _p->fd = (*p)->fd; // change the FD to the new one
            *p = _p; // return the previous state with the current FD
            plrs->players[_p->fd] = _p; // update the player in the array
            plrs->dc_players[i] = NULL; // remove the player from the dc_players
            return 0;
        }
    }
    return 1;
}

struct player* create_player(int fd) {
    struct player* p;

    VALIDATE_FD(fd, NULL, plrs->max_player_count)

    p = malloc(sizeof(struct player));
    if (!p) {
        return NULL;
    }
    memset(p, 0, sizeof(struct player));
    p->fd = fd;
    p->started_keepalive = false;
    p->last_keepalive = 0;
    p->invalid_sends = 0;
    change_player_name(p, "New Player");
    change_state(p, JUST_CONNECTED);
    return p;
}

struct thr_args {
    int fd;
    unsigned keepalive_retry;
};

void* keepalive(void* _p) {
    time_t t;
    struct thr_args* p_args = (struct thr_args*) _p;
    int fd = p_args->fd;
    struct player* p = NULL;
    const unsigned long_term_dc = 60;
    const unsigned short_term_dc = 20; // perhaps add it as a var later on
    double diff;
    time_t last_ka;
    bool short_term_disconnected = false;

    printf("Keepalive started!\n");
    if (lookup_player_by_fd(fd, &p)) {
        printf("Failed to lookup player by FD (fd = %d)\n", fd);
        goto end;
    }

    p->started_keepalive = true;

    time(&t);
    p->last_keepalive = t;
    last_ka = t;

    while (1) {
        // update the current time
        time(&t);

        if (p && !lookup_player_by_fd(p->fd, &p) && p->started_keepalive) {
            last_ka = p->last_keepalive;
        }
        // get the difference in seconds between the last keep alive packets
        diff = difftime(t, last_ka);

        // long term disconnect stops the thread and disconnects the player completely (even from game)
        if (diff >= long_term_dc) {
            printf("The player has not sent a keepalive packet for %us, disconnecting him from game...\n",
                   long_term_dc);
            break;
        }

        // short term disconnect doesn't stop the thread, only proclaims the player as disconnected
        if (diff >= short_term_dc) {
            if (!short_term_disconnected) {
                printf("Could not reach a player %s with FD %d for %us...\n", p->name,
                       p->fd, short_term_dc);
                short_term_disconnected = true;
                disconnect(p, NULL);
            }
        } else {
            short_term_disconnected = false;
        }

        // sleep for one second before checking again
        // reduces overhead
        usleep(500000);
    }
    end:
    // after 30 seconds of timeout we disconnect the player indefinitely
    if (p != NULL) {
        disconnect(p, "Timed out");
        p->started_keepalive = false;
    }
    free(_p);
    return NULL;
}

void start_keepalive(int fd, unsigned keepalive_retry) {
    pthread_t thread;
    struct player* p;
    struct thr_args* args;

    if (lookup_player_by_fd(fd, &p)) {
        return;
    }

    p->last_keepalive = time(NULL);
    if (p->started_keepalive) {
        return;
    }
    args = malloc(sizeof(struct thr_args));
    args->fd = fd;
    args->keepalive_retry = keepalive_retry;

    printf("Starting keepalive for %d...\n", args->fd);
    if (pthread_create(&thread, NULL, keepalive, args)) {
        fprintf(stderr, "Error creating thread\n");
    }
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
    if (!plrs) {
        return 1;
    }
    VALIDATE_FD(fd, 1, plrs->max_player_count)

    // the player under this FD is already connected
    // this should not happen, if this
    // happens some logic inside the
    // server failed
    if (plrs->players[fd]) {
        return 2;
    }

    // add the client to the connected players
    plrs->players[fd] = create_player(fd);
    return plrs->players[fd] != NULL;
}

void change_state(struct player* p, enum player_state ps) {
    char* ps_str;

    if (!p) {
        fprintf(stderr, "Failed to change state, player is null!\n");
        return;
    }
    switch (ps) {
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
            fprintf(stderr, "Attempted to set an invalid player state! (%d)\n", ps);
            return;
    }
    p->ps = ps;
    printf("Changed %s's state to: %s\n", p->name, ps_str);
}

int pman_handle_dc(struct player* p) {
    int i;

    if (!p) {
        return 1;
    }
    VALIDATE_FD(p->fd, 2, plrs->max_player_count)

    plrs->players[p->fd] = NULL;

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
    for (i = 0; i < plrs->max_player_count; ++i) {
        // find first empty spot for the disconnected player
        if (!plrs->dc_players[i]) {
            printf("Storing %s under %d in the disconnected list\n", p->name, i);
            p->fd = -1; // set the FD to a negative value to indicate the player is disconnected
            p->invalid_sends = 0; // reset the invalid sends
            plrs->dc_players[i] = p; // and store the player
            return 0;
        }
    }

    return 1;
}