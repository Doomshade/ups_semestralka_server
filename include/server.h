#ifndef SEMESTRALKA_SERVER_H
#define SEMESTRALKA_SERVER_H

#define __DEBUG_MODE 0

/**
 * Total maximum players active at once
 */
#define MAX_GAME_COUNT 256

#define MAX_PLAYER_COUNT MAX_GAME_COUNT * 2

/**
 * Starts the server on the given port
 * @param port the port
 * @return
 */
int start_server(unsigned port);

#endif //SEMESTRALKA_SERVER_H
