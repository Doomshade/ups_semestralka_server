#ifndef SEMESTRALKA_GAME_MNGR_H
#define SEMESTRALKA_GAME_MNGR_H

#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "player_mngr.h"

/**
 * The default g FEN string
 */
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define BLACK 0 << 0
#define WHITE 1 << 0
#define WIN_BY_MATE 1 << 1
#define WIN_BY_RESIGNATION 1 << 2
#define WIN_BY_TIME 1 << 3
#define DRAW 0

#define CASTLES_BLACK_QUEENSIDE 1 << 0
#define CASTLES_BLACK_KINGSIDE 1 << 1
#define CASTLES_WHITE_QUEENSIDE 1 << 2
#define CASTLES_WHITE_KINGSIDE 1 << 3

// transforms A-F -> 0-7
#define FILE_TO_UINT(file) (file - 'A')

#define UINT_TO_FILE(file) (file + 'A')

// transforms 1-8 (ASCII) -> 0-7
#define RANK_TO_UINT(rank) (rank - '1')
#define UINT_TO_RANK(rank) (rank + '1')


#define INGAME(g, p) (g->white == p || g->black == p)
#define OPPONENT(g, p) (g->white == p ? g->black : g->white)

/**
 * The square on the g
 */
struct square {
    // rank = 1-8 represented as 0-7
    unsigned int rank;
    // file = A-F represented as 0-7
    unsigned int file;
};

/**
 * A move in-game
 */
struct move {
    // the originating square of the move
    struct square* from;
    // the target square
    struct square* to;
};

/**
 * The game
 */
struct game {
    struct player* white;
    struct player* black;

    // the castle flags
    // the flags are:
    // CASTLES_WHITE_KINGSIDE
    // CASTLES_WHITE_QUEENSIDE
    // CASTLES_BLACK_KINGSIDE
    // CASTLES_BLACK_QUEENSIDE
    // 0b1111 means both players can castle both ways whereas
    // 0b0000 means neither player can castle
    int castles;

    // board represented by chars of pieces - "rnbqkpRNBQKP" (r = rook, n = knight, ...).
    // first index is the rank (1-8) and the second is the file (A-F) both represented as 0-7.
    char board[8][8];

    // The last pawn move (because of en passant and FEN string)
    struct square* lm;

    // the number of halfmoves since the last capture or pawn advance, used for the fifty-move rule
    int halfmove_clock;
    // the number of the full move, it starts at 1, and is incremented after black's move
    int fullmove_count;
    // whether the white player is to move
    bool white_to_move;
};

/**
 * Creates a move
 * @param from
 * @param to
 * @return
 */
struct move* create_move(struct square* from, struct square* to);

void free_move(struct move** m);

/**
 * Creates a square
 * @param file
 * @param rank
 * @return the square
 */
struct square* create_square(unsigned int file, unsigned int rank);

/**
 * Sends the player the FEN string and informs the
 * opponent that the player has reconnected to the game.
 * @param pl the player
 * @param g the game
 * @return 0 if the player was last in-game and everything went alright
 */
int reconnect_to_game(struct player* pl, struct game* g);

/**
 * Generates the FEN string for the given game
 * @param g the game
 * @return the FEN string
 */
char* generate_fen(struct game* g);

/**
 * Informs the player that the opponent has disconnected
 * @param p the player to inform
 * @return 0 if everything went alright
 */
int gman_handle_dc(struct player* p);

/**
 * Sets up the game from the fen string
 * @param fen the fen string
 * @return 0 if everything went alright
 */
int setup_game(struct game* g);

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
 * @param winner
 * 1st bit (who won):\n
 * (0) white\n
 * (1) black\n\n
 * 2-3 bits (game finished by):\n
 * (00) mate\n
 * (01) resignation\n
 * (10) time\n
 * (11) draw\n
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
int move_piece(struct game* g, struct player* p, struct move* m);

/**
 * Attempts to lookup a game by player's name
 * @param name the player's name
 * @param p the player
 * @return the game if it exists or NULL
 */
struct game* lookup_game(struct player* p);

#endif //SEMESTRALKA_GAME_MNGR_H
