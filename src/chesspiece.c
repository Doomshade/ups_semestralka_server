#include "../include/chesspiece.h"
#include <ctype.h>

#define HANDLER_SIZE 'z' // yes we define the max size to the last char in the alphabet because we can
#define ADD_HANDLE(piece, handle) move_handlers[TO_WHITE(piece)] = handle; move_handlers[TO_BLACK(piece)] = handle;
// e. g. this transposes [4, 0] to 32, [2, 1] to 17
// this allows us to check the board matrix by offsetting it
#define SQUARE_TO_INT(rank, file) (rank * 8 + file)
#define IN_BOUNDS(a) (a < 8 && a > 0)

typedef int move_handle(struct game* g, struct move* m);

move_handle** move_handlers = NULL;

int move(char piece, struct game* g, struct move* m) {
    if (piece >= HANDLER_SIZE || piece < 0) {
        return MOVE_INVALID;
    }
    return move_handlers[piece](g, m);
}

// the direction for all diagonals/horizontal or vertical moves should be defined as: 11111111 (queen)
// 111
// 101
// 111
// pawn should look like: 11100000
// 111
// 000
// 000
// the middle bit is out of convenience
long long gen_pos_moves(struct game* g, struct square* from, int direction, unsigned int amount) {
    unsigned int rank, file;
    long long bitmap = 0;
    char piece;
    unsigned int a; // the amount copy to reset it every time we reach the end of search
    int vx, vy;
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

    //
    for (; direction != 0; direction >>= 1) {
        // the piece does not move in this direction
        if (!(direction & 1)) {
            continue;
        }
        vx = 1 - (direction % 3);
        vy = -1 + (direction / 3);

        // it's possible that sb accidentally set the middle bit to 1, skip it
        if (vx == 0 && vy == 0) {
            continue;
        }


        for (rank = from->rank + vx, file = from->file + vy, a = amount; // start at the offset, and reset the amount
             IN_BOUNDS(rank) && IN_BOUNDS(file) && a > 0; // check for the amount, and whether we are in bounds
             rank += vx, file += vy, --a) // iterate in the direction, and reduce the amount by 1
        {
            piece = PIECE(g->board, rank, file);

            // empty square is ok
            if (IS_EMPTY_SQUARE(piece)) {
                bitmap |= 1 << SQUARE_TO_INT(rank, file);
                continue;
            }

            if (IS_WHITE(piece) != curr_white) {
                bitmap |= 1 << SQUARE_TO_INT(rank, file);
            }
            // there is a piece, stop right there (you criminal scum!!!)
            break;
        }
    }
    return bitmap;
}

int is_pos_move(long long pos_moves, struct square* sq) {
    int i;
    unsigned int rank, file;
    if (!sq) {
        return MOVE_INVALID;
    }
    for (i = 0; i < 64; ++i, pos_moves >>= 1) {
        // the move is not valid
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

int pawn_handle(struct game* g, struct move* m) {
    int direction;
    long long pos_moves;
    if (!g || !m) {
        return MOVE_INVALID;
    }
    direction = IS_WHITE(PIECE_SQ(g->board, m->from)) ? 0b111000000 : 0b000000111;
    pos_moves = gen_pos_moves(g, m->from, direction, 1);
    if (is_pos_move(pos_moves, m->to) == MOVE_VALID) {
        return MOVE_VALID;
    }
    return MOVE_INVALID;
}

int rook_handle(struct game* g, struct move* m) {
    return MOVE_INVALID;
}

int knight_handle(struct game* g, struct move* m) {
    return MOVE_INVALID;
}

int bishop_handle(struct game* g, struct move* m) {
    return MOVE_INVALID;
}

int king_handle(struct game* g, struct move* m) {
    return MOVE_INVALID;
}

int queen_handle(struct game* g, struct move* m) {
    return MOVE_INVALID;
}


void init_cpce() {
    if (move_handlers) {
        return;
    }
    move_handlers = calloc(HANDLER_SIZE, sizeof(move_handle*));
    ADD_HANDLE(PAWN, pawn_handle)
    ADD_HANDLE(ROOK, rook_handle)
    ADD_HANDLE(KNIGHT, knight_handle)
    ADD_HANDLE(BISHOP, bishop_handle)
    ADD_HANDLE(KING, king_handle)
    ADD_HANDLE(QUEEN, queen_handle)
}
