#ifndef SEMESTRALKA_GAME_MNGR_H
#define SEMESTRALKA_GAME_MNGR_H

#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>

/**
 * The default board FEN string
 */
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define WHITE_WINNER 0
#define BLACK_WINNER 1
#define WIN_BY_MATE 0b0
#define WIN_BY_RESIGNATION 0b10
#define WIN_BY_TIME 0b100

/**
 * The chessboard
 */
struct chessboard {
    /**
     * Board represented by chars of pieces - "rnbqkpRNBQKP" (r = rook, n = knight, ...)
     */
    char board[8][8];

    /**
     * The last pawn move (because of en passant and FEN string)
     */
    char lm[2];
};

/**
 * The game
 */
struct game {
    struct player* white;
    struct player* black;
    struct chessboard* board;
    bool white_to_move;
};

int reconnect_to_game(struct player* pl, struct game* g);

const char* generate_fen(struct chessboard* board);

/**
 * Sets up the game from the fen string
 * @param fen the fen string
 * @return 0 if everything went alright
 */
int setup_game(struct game* g, char* fen);

/**
 * Creates a new game for the players,
 * puts them into the PLAY state, and
 * sends a packet to both players that
 * they received a queue
 * @param white
 * @param black
 * @return
 */
int game_create(struct player* white, struct player* black);

/**
 * Finishes the game, frees it, sends the packets
 * the players, informing them of the fact, and
 * changes their state to LOGGED_IN.
 * @param g the game
 * @param winner -1 if draw,
 * 0 if white by mate,
 * 1 if black by mate,
 * 2 if white by resignation,
 * 3 if black by resignation,
 * 4 if white by time,
 * 5 if black by time
 */
int finish_game(struct game* g, int winner);

/**
 * Sets up the game manager
 */
void init_gman();

/**
 * A move invoked by the player
 * @param p the player
 * @param file_from the file_from
 * @param rank_from the rank_from
 * @return 0 if everything went fine
 */
int move_piece(struct game* g, struct player* p, unsigned int rank_from, unsigned int file_from, unsigned int rank_to, unsigned int file_to);

/**
 * Attempts to lookup a game by player's name
 * @param name the player's name
 * @param p the player
 * @return the game if it exists or NULL
 */
struct game* lookup_game(struct player* p);

#endif //SEMESTRALKA_GAME_MNGR_H
