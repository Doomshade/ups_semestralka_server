#ifndef SEMESTRALKA_SERVER_H
#define SEMESTRALKA_SERVER_H

/**
 * Total maximum players active at once
 */
#define MAX_GAME_COUNT 256

#define MAX_PLAYER_COUNT MAX_GAME_COUNT * 2

int start_server(unsigned port);

#endif //SEMESTRALKA_SERVER_H
