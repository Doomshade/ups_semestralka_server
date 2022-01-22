#include "../include/chesspiece.h"
#include <stdio.h>

#define HANDLER_SIZE 'z' // yes we define the max size to the last char in the alphabet because we can
#define ADD_HANDLE(piece, handle) move_handlers[TO_WHITE(piece)] = handle; move_handlers[TO_BLACK(piece)] = handle;
// e. g. this transposes [4, 0] to 32, [2, 1] to 17
// this allows us to check the board matrix by offsetting it
#define SQUARE_TO_INT(rank, file) (rank * 8 + file)
#define IN_BOUNDS(num) (num <= 7 && num >= 0)
#define VALIDATE_PARAMS(g, m) if (!g || !m) return MOVE_INVALID;
#define ABS(n) (signed) (n) < 0 ? -(n) : (n)

typedef int move_handle(struct game* g, struct move* m);

move_handle** move_handlers = NULL;

int move(char piece, struct game* g, struct move* m) {
    if (piece >= HANDLER_SIZE || piece < 0) {
        return MOVE_INVALID;
    }
    return move_handlers[piece](g, m);
}

// the direction for all diagonals/horizontal or vertical moves should be defined as: 111101111 (queen)
// 111
// 101
// 111
// a pawn should look like: 11100000 or 000000111 - white or black, respectively
// 111
// 000
// 000
// the middle bit is useless, but remains there out of convenience
unsigned long long int gen_pos_moves(struct game* g, struct square* from, int direction, unsigned int amount) {
    unsigned long long offset;
    unsigned int rank, file;
    unsigned long long int bitmap = 0;
    char piece;
    unsigned int a; // the amount copy to reset it every time we reach the end of search
    int vx, vy;
    int i;
    bool curr_white;

    // the direction should not be larger than 2^9 - 1
    if (direction >= 512) {
        return 0;
    }

    curr_white = IS_WHITE(PIECE_SQ(g->board, from));
    // we start from the bottom-right (the least significant bit)
    // 0b000000001 = [ 1, -1] direction = 0
    // 0b000000010 = [ 0, -1] direction = 1
    // 0b000000100 = [-1, -1] direction = 2
    // 0b000001000 = [ 1,  0] direction = 3
    // 0b000010000 = [ 0,  0] direction = 4
    // 0b000100000 = [-1,  0] direction = 5
    // 0b001000000 = [ 1,  1] direction = 6
    // 0b010000000 = [ 0,  1] direction = 7
    // 0b100000000 = [-1,  1] direction = 8

    for (i = 0; direction != 0; direction >>= 1, ++i) {
        // the piece does not move in this direction
        if (!(direction & 1)) {
            continue;
        }

        // the piece moves in that direction, calculate the x and y on the board
        vx = -1 + (i / 3);
        vy = 1 - (i % 3);

        // it's possible that we accidentally set the middle bit to 1, skip it
        if (vx == 0 && vy == 0) {
            continue;
        }


        for (rank = from->rank + vx, file = from->file + vy, a = amount; // start at the offset, and reset the amount
                ;
             rank += vx, file += vy, --a) // iterate in the direction, and reduce the amount by 1
        {
            // if the move is out of bounds or the amount is 0, stop checking
            if (!(IN_BOUNDS(rank) && IN_BOUNDS(file) && a > 0)) {
                printf("%d %d %d\n", IN_BOUNDS(rank), IN_BOUNDS(file), a);
                break;
            }

            // we can continue, get the piece on the square
            piece = PIECE(g->board, rank, file);

            // calculate the offset in the bitmap
            offset = SQUARE_TO_INT(rank, file);

            // empty square is ok, store it
            if (IS_EMPTY_SQUARE(piece)) {
                bitmap |= (1ULL << offset);
                continue;
            }

            // an enemy piece is on the square, add it as well
            if (IS_WHITE(piece) != curr_white) {
                bitmap |= (1ULL << offset);
            }
            // but there is a piece, stop right there (you criminal scum!!!)
            break;
        }
    }
    // we return a 64-bit bitmap representing every square on the board
    // all 0's mean there's no valid move, all 1's mean all moves are valid
    return bitmap;
}

/**
 * Checks whether the given square is a valid move with the bitmap
 * @param pos_moves the bitmap of possible moves on the board
 * @param sq the target square
 * @return MOVE_VALID if the move is valid, MOVE_INVALID otherwise
 */
int is_pos_move(unsigned long long int pos_moves, struct square* sq) {
    int i;
    unsigned int rank, file;

    if (!sq) {
        return MOVE_INVALID;
    }

    // iterate over the bits in the pos_moves
    // shift the bit after each iteration
    for (i = 0; i < 64 && pos_moves != 0; ++i, pos_moves >>= 1) {
        // the move is not in the bitmap, skip over it
        if (!(pos_moves & 1)) {
            continue;
        }
        rank = i / 8;
        file = i % 8;
        if (sq->rank == rank && sq->file == file) {
            return MOVE_VALID;
        }
    }
    return MOVE_INVALID;
}

// this macro checks whether the "f" file and "r" rank can be played
// with the knight move "m" in game "g"
#define RETURN_IF_VALID(f, r, m, g) \
if (IN_BOUNDS((r)) && IN_BOUNDS((r)) &&\
(m)->to->rank == ((r)) && (m)->to->file == (f)) {\
if (PIECE_SQ((g)->board, (m)->to) == EMPTY_SQUARE) {\
return MOVE_VALID;\
}\
if (IS_WHITE(PIECE_SQ((g)->board, (m)->to)) !=\
IS_WHITE(PIECE_SQ((g)->board, (m)->from))) {\
return MOVE_VALID;\
}\
}


int knight_handle(struct game* g, struct move* m) {
    int i;
    unsigned int file;
    unsigned int rank;
    unsigned int file_off; // the file with offset value
    unsigned int rank_off; // the rank with offset value
    VALIDATE_PARAMS(g, m)

    file = m->from->file;
    rank = m->from->rank;
    for (i = 0; i < 4; ++i) {
        // [-2 -1]
        // [-2 +1]
        // [+2 -1]
        // [+2 +1]
        file_off = (i & 0b10 ? -2 : +2) + file;
        rank_off = (i & 0b01 ? -1 : +1) + rank;

        RETURN_IF_VALID(file_off, rank_off, m, g)

        // [-1 -2]
        // [-1 +2]
        // [+1 -2]
        // [+1 +2]
        file_off = (i & 0b10 ? -1 : +1) + file;
        rank_off = (i & 0b01 ? -2 : +2) + rank;

        RETURN_IF_VALID(file_off, rank_off, m, g)
    }
    return MOVE_INVALID;
}

int handle_simple_piece(struct game* g, struct move* m, int direction, int amount) {
    unsigned long long int pos_moves;
    VALIDATE_PARAMS(g, m)

    pos_moves = gen_pos_moves(g, m->from, direction, amount);
    return is_pos_move(pos_moves, m->to);
}

int king_handle(struct game* g, struct move* m) {
    char king;
    int white;
    int ret = 0;
    unsigned int diff;

    VALIDATE_PARAMS(g, m)
    king = PIECE_SQ(g->board, m->from);
    white = IS_WHITE(king);

    // the move is castles as the king was on the E file, starting rank, and attempted to move by two files on the same rank
    diff = ABS(m->from->file - m->to->file);
    if (m->from->file == 4 && diff == 2 && m->from->rank == m->to->rank &&
        (m->from->rank == 0 || m->from->rank == 7)) {
        printf("Player wants to castle\n");
        ret = MOVE_CASTLES;
        // long castles
        if (m->to->file == 2) {
            // check if all the squares are empty
            if (!IS_EMPTY_SQUARE(g->board[m->from->rank][m->from->file - 1]) ||
                !IS_EMPTY_SQUARE(g->board[m->from->rank][m->from->file - 2]) ||
                !IS_EMPTY_SQUARE(g->board[m->from->rank][m->from->file - 3])) {
                return MOVE_INVALID;
            }
            ret |= MOVE_LONG_CASTLES;
            // white or black or neither
            if (m->from->rank == 0 && white && g->castles & CASTLES_WHITE_QUEENSIDE) {
                ret |= MOVE_WHITE_CASTLES;
            } else if (m->from->rank == 7 && !white && g->castles & CASTLES_BLACK_QUEENSIDE) {
            } else {
                return MOVE_INVALID;
            }
        }

            // short castles
        else if (m->to->file == 6) {
            // check if all the squares are empty
            if (!IS_EMPTY_SQUARE(g->board[m->from->rank][m->from->file + 1]) ||
                !IS_EMPTY_SQUARE(g->board[m->from->rank][m->from->file + 2])) {
                return MOVE_INVALID;
            }
            // white or black or neither
            if (m->from->rank == 0 && white && g->castles & CASTLES_WHITE_QUEENSIDE) {
                ret |= MOVE_WHITE_CASTLES;
            } else if (m->from->rank == 7 && !white && g->castles & CASTLES_BLACK_QUEENSIDE) {
            } else {
                return MOVE_INVALID;
            }
        }
            // this can't happen, but why not
        else {
            return MOVE_INVALID;
        }
    }
    // modify castles privileges
    // if the king is white, remove castles privileges from white,
    // otherwise from black
    g->castles &= ~(white ?
                    CASTLES_WHITE_KINGSIDE | CASTLES_WHITE_QUEENSIDE :
                    CASTLES_BLACK_KINGSIDE | CASTLES_BLACK_QUEENSIDE);

    // ret isn't 0 -> that means the move is castles
    if (ret != 0) {
        return ret;
    }

    // the move isn't castles, handle a simple movement
    return handle_simple_piece(g, m, 0b111101111, 1);
}

int pawn_handle(struct game* g, struct move* m) {
    int capturing_direction;
    int moving_direction;
    char piece;
    bool white;
    VALIDATE_PARAMS(g, m)
    white = IS_WHITE((unsigned char) PIECE_SQ(g->board, m->from));

    // the move was en passant
    if (g->lm && m->to->rank == g->lm->rank && m->to->file == g->lm->file) {
        return MOVE_EN_PASSANT;
    }

    if (m->from->file == m->to->file) {

        // we check for two squares first as it's faster to compute
        // check if the pawn is on the 2nd rank for white and the move is by two
        if ((m->from->rank == 1 && white && m->to->rank == 3)) {
            // check if there are two empty squares above
            if (!IS_EMPTY_SQUARE(g->board[m->from->rank + 1][m->from->file]) ||
                !IS_EMPTY_SQUARE(g->board[m->from->rank + 2][m->from->file])) {
                return MOVE_INVALID;
            }
            // because the pawn moved by two squares, update the last move
            if (g->lm) {
                free(g->lm);
            }
            g->lm = create_square(m->from->file, m->from->rank + 1);
            return MOVE_VALID;
        }
            // check if the pawn is on the 7th rank for black and the move is by two
        else if (m->from->rank == 6 && !white && m->to->rank == 4) {
            if (!IS_EMPTY_SQUARE(g->board[m->from->rank - 1][m->from->file]) ||
                !IS_EMPTY_SQUARE(g->board[m->from->rank - 2][m->from->file])) {
                return MOVE_INVALID;
            }
            // because the pawn moved by two squares, update the last move
            if (g->lm) {
                free(g->lm);
            }
            g->lm = create_square(m->from->file, m->from->rank - 1);
            return MOVE_VALID;
        }
    }


    // first handle the attacking moves
    // these are the one square taking moves for the pawns
    capturing_direction = white ? 0b101000000 : 0b000000101;
    moving_direction = white ? 0b010000000 : 0b000000010;
    piece = PIECE(g->board, m->to->rank, m->to->file);

    // check if the move was captures and there was an ENEMY piece
    if (handle_simple_piece(g, m, capturing_direction, 1) == MOVE_VALID && piece != EMPTY_SQUARE &&
        IS_WHITE(piece) != white) {
        if (g->lm) {
            free(g->lm);
            g->lm = NULL;
        }
        return MOVE_VALID;
    }

    // check if the move was by one square and the square was empty
    if (handle_simple_piece(g, m, moving_direction, 1) == MOVE_VALID && piece == EMPTY_SQUARE) {
        if (g->lm) {
            free(g->lm);
            g->lm = NULL;
        }
        return MOVE_VALID;
    }
    return MOVE_INVALID;
}

int rook_handle(struct game* g, struct move* m) {
    return handle_simple_piece(g, m, 0b010101010, 8);
}

int bishop_handle(struct game* g, struct move* m) {
    return handle_simple_piece(g, m, 0b101000101, 8);
}

int queen_handle(struct game* g, struct move* m) {
    return handle_simple_piece(g, m, 0b111101111, 8);
}


void init_cpce() {
    if (move_handlers) {
        return;
    }
    printf("Initializing chess piece handlers...\n");
    move_handlers = calloc(HANDLER_SIZE, sizeof(move_handle*));
    ADD_HANDLE(PAWN, pawn_handle)
    ADD_HANDLE(ROOK, rook_handle)
    ADD_HANDLE(KNIGHT, knight_handle)
    ADD_HANDLE(BISHOP, bishop_handle)
    ADD_HANDLE(KING, king_handle)
    ADD_HANDLE(QUEEN, queen_handle)
}
