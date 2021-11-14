#include <string.h>
#include <stdio.h>
#include "../include/player_mngr.h"
#include "../include/server.h"
#include "../include/game_mngr.h"

#define VALIDATE_FD(fd, ret) if (fd >= MAX_PLAYER_COUNT) return ret;
struct player* players[MAX_PLAYER_COUNT] = {0};
struct player* dc_players[MAX_PLAYER_COUNT] = {0};

int lookup_player_by_fd(int fd, struct player** p) {
    struct player* _p;

    VALIDATE_FD(fd, 2)
    _p = players[fd];
    if (_p) {
        *p = _p;
        return 0;
    }
    return 1;
}

int lookup_player_by_name(char* name, struct player** p) {
    int i;
    struct player* _p;

    for (i = 0; i < MAX_PLAYER_COUNT; ++i) {
        _p = players[i];
        if (_p == NULL) {
            continue;
        }

        // the player was found (he disconnected)
        if (strcmp(_p->name, name) == 0) {

            return 0;
        }
    }
    return 1;
}

struct player* create_player(int fd, char* name) {
    struct player* p;

    VALIDATE_FD(fd, NULL)
    if (!name) {
        return NULL;
    }

    p = malloc(sizeof(struct player));
    if (!p) {
        return NULL;
    }
    p->fd = fd;
    p->name = "New Player";
    change_state(p, JUST_CONNECTED);
    return p;
}

int handle_new_connection(int fd) {
    VALIDATE_FD(fd, 1)

    // this should not happen, if this
    // happens some logic inside the
    // server failed
    if (players[fd]) {
        return 2;
    }

    // add the client to the connected players
    players[fd] = create_player(fd, "");
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

int handle_possible_reconnection(struct player** p) {
    int i;
    struct game* g;
    if (p == NULL || *p == NULL) {
        return 1;
    }
    VALIDATE_FD((*p)->fd, 1)

    for (i = 0; i < MAX_PLAYER_COUNT; ++i) {
        // the player has previously disconnected, copy the data
        // from the disconnected instance to the new one EXCEPT
        // for the FD (and the name can be skipped too)
        if (strcmp(dc_players[i]->name, (*p)->name) == 0) {
            change_state(*p, dc_players[i]->ps);

            // TODO
            // the client was in-game, lookup the game and reconnect him
            if ((*p)->ps == PLAY) {
                g = lookup_game(*p);

                // the game has likely ended
                if (!g) {
                    return 1;
                }

            }
        }
    }
    return 0;
}

int handle_disconnection(int fd) {
    int i;
    struct player* p;

    VALIDATE_FD(fd, 2)

    // look up the player by his current FD before disconnection
    // and remove him from the connected list
    for (i = 0; i < MAX_PLAYER_COUNT; ++i) {
        if (!players[i]) {
            continue;
        }
        if (players[i]->fd == fd) {
            p = players[i];
            players[i] = NULL; // remove him from the player list, add him to the disconnected one
        }
    }

    // player not found, ignore it
    if (!p) {
        return 1;
    }

    // add him to the disconnected list
    for (i = 0; i < MAX_PLAYER_COUNT; ++i) {
        if (dc_players[i] == NULL) {
            dc_players[i] = p;
            return 0;
        }
    }

    return 1;
}