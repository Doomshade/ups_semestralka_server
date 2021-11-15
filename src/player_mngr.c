#include <string.h>
#include <stdio.h>
#include "../include/player_mngr.h"
#include "../include/server.h"
#include "../include/game_mngr.h"

#define MAX_DISCONNECTED_COUNT MAX_PLAYER_COUNT * 5

#define VALIDATE_FD(fd, ret) if (fd >= MAX_PLAYER_COUNT) return ret;

struct player* players[MAX_PLAYER_COUNT] = {0};
struct player* dc_players[MAX_DISCONNECTED_COUNT] = {0};

int lookup_player_by_fd(int fd, struct player** p) {
    struct player* _p;

    VALIDATE_FD(fd, 2)
    if (!p) {
        return 1;
    }
    _p = players[fd];
    if (_p) {
        *p = _p;
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
        if (strcmp(_p->name, name) == 0) {
            *p = _p;
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
    p->fd = fd;
    change_player_name(p, "New Player");
    change_state(p, JUST_CONNECTED);
    return p;
}

int change_player_name(struct player* p, char* name) {
    if (!p || !name) {
        return 2;
    }
    if (strlen(name) > MAX_PLAYER_NAME_LENGTH) {
        return 1;
    }
    strcpy(p->name, name);
    return 0;
}

int handle_new_connection(int fd) {
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

int handle_disconnection(int fd) {
    int i;
    struct player* p;
    int ret;

    VALIDATE_FD(fd, 2)

    // the player was not found
    if ((ret = lookup_player_by_fd(fd, &p))) {
        return ret;
    }

    // player not found, ignore it
    if (!p) {
        return 1;
    }

    players[fd] = NULL;


    // add him to the disconnected list
    // CAUTION: do not store it under the
    // clients FD! a situation like:
    // a client connects with an FD of 4,
    // joins a game, disconnects,
    // a different client joins and is given
    // the FD 4 and that client would be reconnected!
    for (i = 0; i < MAX_PLAYER_COUNT; ++i) {
        if (dc_players[i] == NULL) {
            dc_players[i] = p;
            return 0;
        }
    }

    return 1;
}