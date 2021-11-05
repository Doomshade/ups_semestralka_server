#ifndef SEMESTRALKA_GAME_MNGR_H
#define SEMESTRALKA_GAME_MNGR_H

#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>

/**
 * The default board FEN string
 */
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

enum player_state {
    /**
     * The initial state -- when the player connects to the server
     */
    JUST_CONNECTED,

    /**
     * The player is in a queue
     */
    QUEUE,

    /**
     * The player is in a game and white is to move
     */
    WHITE_TO_MOVE,

    /**
     * The player is in a game and black is to move
     */
    BLACK_TO_MOVE
};

struct player {

    /**
     * The current state of the player
     */
    enum player_state ps;

    /**
     * The file descriptor of this player
     */
    int fd;

    /**
     * The name of this player
     */
    char* name;
};

struct chessboard {
    /**
     * Board represented by chars of pieces - "rnbqkpRNBQKP" (r = rook, n = knight, ...)
     */
    char board[8][8];

    /**
     * The last pawn move (because of en passant and FEN string)
     */
    char lpm[2];
};


struct game {
    struct player* white;
    struct player* black;
    struct chessboard* board;

    /**
     * Tells whether white is to move
     */
    bool white_to_move;
};

int setup_game(char* fen);


struct player* create_player(int fd, char* name);

struct game* lookup_game(char* name, struct player** p);

struct game* create_game(struct player* white, struct player* black, bool white_to_move);

void free_player(struct player* p);

void free_game(struct game* g);


#endif //SEMESTRALKA_GAME_MNGR_H
