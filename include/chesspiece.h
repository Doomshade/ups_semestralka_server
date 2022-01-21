#ifndef SEMESTRALKA_CHESSPIECE_H
#define SEMESTRALKA_CHESSPIECE_H

#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <ctype.h>
#include "game_mngr.h"

#define PAWN 'P' // white pawn
#define ROOK 'R' // white rook
#define KNIGHT 'N' // white knight
#define BISHOP 'B' // white bishop
#define KING 'K' // white king
#define QUEEN 'Q' // white queen

#define EMPTY_SQUARE ' '
#define IS_EMPTY_SQUARE(square) (square == EMPTY_SQUARE)

#define TO_WHITE(piece) (toupper(piece))
#define TO_BLACK(piece) (tolower(piece))
#define IS_WHITE(piece) (TO_WHITE(piece) == piece)

#define IS_PAWN(square) (TO_WHITE(square) == PAWN)
#define IS_ROOK(square) (TO_WHITE(square) == ROOK)
#define IS_KNIGHT(square) (TO_WHITE(square) == KNIGHT)
#define IS_BISHOP(square) (TO_WHITE(square) == BISHOP)
#define IS_KING(square) (TO_WHITE(square) == KING)
#define IS_QUEEN(square) (TO_WHITE(square) == QUEEN)

#define MOVE_VALID 0
#define MOVE_CASTLES 1 << 1
#define MOVE_WHITE_CASTLES 1 << 2
#define MOVE_LONG_CASTLES 1 << 3
#define MOVE_EN_PASSANT 1 << 4
#define MOVE_INVALID -1

#define PIECE(board, rank, file) board[rank][file]
#define PIECE_SQ(board, square) PIECE(board, square->rank, square->file)

/**
 * Initializes the chess piece packet_handlers
 */
void init_cpce();

/**
 * Attempts to move the piece on the board
 * @param piece the piece
 * @param g the game
 * @param m the move
 * @return MOVE_ type
 */
int move(char piece, struct game* g, struct move* m);

#endif //SEMESTRALKA_CHESSPIECE_H
