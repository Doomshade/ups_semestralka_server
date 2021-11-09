#include <string.h>
#include "../include/player_mngr.h"
#include "../include/server.h"
#include "../include/game_mngr.h"

#define VALIDATE_FD(fd) if (fd >= MAX_PLAYER_COUNT)
struct player* players[MAX_PLAYER_COUNT] = {0};
struct player* dc_players[MAX_PLAYER_COUNT] = {0};

int lookup_player(char* name, struct player** p) {
    int i;
    struct player* _p;

    for (i = 0; i < MAX_PLAYER_COUNT; ++i) {
        _p = players[i];
        if (_p == NULL) {
            continue;
        }

        // the player was found (he disconnected)
        if (strcmp(_p->name, name) == 0) {

            // change his previous fd to his new fd
            _p->fd = (*p)->fd;

            // change his current state to the previous state
            (*p)->ps = _p->ps;

            // remove him from the
            *p = _p;
            return 0;
        }
    }
    return 1;
}

struct player* create_player(int fd, char* name) {

    VALIDATE_FD(fd) return NULL;
    if (name == NULL) {
        return NULL;
    }
    struct player* p = malloc(sizeof(struct player));
    if (p == NULL) {
        return NULL;
    }
    p->fd = fd;
    p->ps = JUST_CONNECTED;
    p->name = name;
    return p;
}

int handle_new_connection(int fd, struct player** p) {
    int i;
    struct player* _p;
    if (p == NULL) {
        return 1;
    }

    VALIDATE_FD(fd) return 1;

    // add him to the connected players
    for (i = 0; i < MAX_PLAYER_COUNT; ++i) {
        if (players[i] == NULL) {
            _p = create_player(fd, "");
            players[i] = _p;
        }
    }
    if (_p == NULL) {
        return 1;
    }
    *p = _p;
    return 0;
}

int handle_disconnection(int fd) {
    int i;
    struct player* p;

    VALIDATE_FD(fd) return 1;

    // look up the player by his current FD before disconnection
    // and remove him from the connected list
    for (i = 0; i < MAX_PLAYER_COUNT; ++i) {
        if (players[i]->fd == fd) {
            p = players[i];
            p->ps = DISCONNECTED;
            players[i] = NULL;
        }
    }
    if (p == NULL) {
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