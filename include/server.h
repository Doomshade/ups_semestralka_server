#ifndef SEMESTRALKA_SERVER_H
#define SEMESTRALKA_SERVER_H

#include <netinet/in.h>
#include "player_mngr.h"

// used by main to communicate with parse_opt.
struct arguments {

    // the IP
    char ip[INET_ADDRSTRLEN];

    // the port
    unsigned port;

    // the limit in seconds until the player is disconnected
    unsigned keepalive_retry;

    // maximum invalid messages send
    unsigned max_inval_pc;

    // whether the server is in debug mode
    unsigned debug_mode;

    // the player limit
    unsigned player_limit;
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
