#ifndef SEMESTRALKA_SERVER_H
#define SEMESTRALKA_SERVER_H

#include <netinet/in.h>
#include "player_mngr.h"

/**
 * Total maximum players active at once
 */
#define MAX_GAME_COUNT 256

#define MAX_PLAYER_COUNT MAX_GAME_COUNT * 2

// used by main to communicate with parse_opt.
struct arguments {
    char ip[INET_ADDRSTRLEN];
    unsigned port;
    unsigned keepalive_retry;
    unsigned max_inval_pc;
    unsigned debug_mode;
};

/**
 * Starts the server on the given port
 * @param ip the IP
 * @param port the port
 * @return
 */
int start_server(struct arguments* args);

/**
 * Disconnects the player from the server
 * @param p the player
 * @param client_socks the fd set of clients
 */
void disconnect(struct player* p, const char* reason);


#endif //SEMESTRALKA_SERVER_H
