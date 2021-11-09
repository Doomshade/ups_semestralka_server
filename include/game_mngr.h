#ifndef SEMESTRALKA_GAME_MNGR_H
#define SEMESTRALKA_GAME_MNGR_H

#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>

/**
 * The default board FEN string
 */
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

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

int lookup_player(char* name, struct player** p);

struct player* create_player(int fd, char* name);

struct game* lookup_game(char* name, struct player** p);

struct game* create_game(struct player* white, struct player* black, bool white_to_move);

void free_player(struct player* p);

void free_game(struct game* g);


#endif //SEMESTRALKA_GAME_MNGR_H
