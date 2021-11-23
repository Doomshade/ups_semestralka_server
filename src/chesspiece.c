#include "../include/chesspiece.h"
#include <stdio.h>

#define HANDLER_SIZE 'z' // yes we define the max size to the last char in the alphabet because we can
#define ADD_HANDLE(piece, handle) move_handlers[TO_WHITE(piece)] = handle; move_handlers[TO_BLACK(piece)] = handle;
// e. g. this transposes [4, 0] to 32, [2, 1] to 17
// this allows us to check the board matrix by offsetting it
#define SQUARE_TO_INT(rank, file) (rank * 8 + file)
#define IN_BOUNDS(num) (num <= 7 && num >= 0)
#define VALIDATE_PARAMS(g, m) if (!g || !m) return MOVE_INVALID;

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
// a pawn should look like: 11100000 or 000000111 - white or black respectively
// 111
// 000
// 000
// the middle bit is out of convenience
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

    //
    for (i = 0; direction != 0; direction >>= 1, ++i) {
        // the piece does not move in this direction
        if (!(direction & 1)) {
            continue;
        }
        vx = -1 + (i / 3);
        vy = 1 - (i % 3);

        // it's possible that sb accidentally set the middle bit to 1, skip it
        if (vx == 0 && vy == 0) {
            continue;
        }


        for (rank = from->rank + vx, file = from->file + vy, a = amount; // start at the offset, and reset the amount
                ; // check for the amount, and whether we are in bounds
             rank += vx, file += vy, --a) // iterate in the direction, and reduce the amount by 1
        {
            if (!(IN_BOUNDS(rank) && IN_BOUNDS(file) && a > 0)) {
                printf("%d %d %d\n", IN_BOUNDS(rank), IN_BOUNDS(file), a);
                break;
            }
            piece = PIECE(g->board, rank, file);

            // empty square is ok
            offset = SQUARE_TO_INT(rank, file);
            if (IS_EMPTY_SQUARE(piece)) {
                bitmap |= (1ULL << offset);
                continue;
            }

            if (IS_WHITE(piece) != curr_white) {
                bitmap |= (1ULL << offset);
            }
            // there is a piece, stop right there (you criminal scum!!!)
            break;
        }
    }
    return bitmap;
}

int is_pos_move(unsigned long long int pos_moves, struct square* sq) {
    int i;
    unsigned int rank, file;
    if (!sq) {
        return MOVE_INVALID;
    }

    // iterate over the bits in the pos_moves
    for (i = 0; i < 64 && pos_moves != 0; ++i, pos_moves >>= 1) {
        // the move in the bitmap
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

int knight_handle(struct game* g, struct move* m) {
    int i;
    unsigned int file;
    unsigned int rank;
    unsigned int af; // the file with offset value
    unsigned int ar; // the rank with offset value
    VALIDATE_PARAMS(g, m)

    file = m->from->file;
    rank = m->from->rank;
    for (i = 0; i < 4; ++i) {
        // [-2 -1]
        // [-2 +1]
        // [+2 -1]
        // [+2 +1]
        af = (i & 0b10 ? -2 : +2) + file;
        ar = (i & 0b01 ? -1 : +1) + rank;

        if (IN_BOUNDS(af) && IN_BOUNDS(ar) &&
            m->to->rank == ar && m->to->file == af) {
            return MOVE_VALID;
        }

        // [-1 -2]
        // [-1 +2]
        // [+1 -2]
        // [+1 +2]
        af = (i & 0b10 ? -1 : +1) + file;
        ar = (i & 0b01 ? -2 : +2) + rank;

        if (IN_BOUNDS(af) && IN_BOUNDS(ar) &&
            m->to->rank == ar && m->to->file == af) {
            return MOVE_VALID;
        }
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

    VALIDATE_PARAMS(g, m)

    // TODO add castles
    return handle_simple_piece(g, m, 0b111101111, 1);
}

int pawn_handle(struct game* g, struct move* m) {
    int direction;
    unsigned int i;
    bool white;
    VALIDATE_PARAMS(g, m)
    white = IS_WHITE((unsigned char) PIECE_SQ(g->board, m->from));

    // the move was en passant
    if (g->lm && m->to->rank == g->lm->rank && m->to->file == g->lm->file) {
        return MOVE_VALID;
    }

    // we check for two squares first as it's faster to compute
    // check if the pawn is on the 2nd rank for white and the move is by two
    if ((m->from->rank == 1 && white && m->to->rank == 3)) {
        // check if there are two empty squares above
        if (!IS_EMPTY_SQUARE(g->board[m->from->rank + 1][m->from->file]) ||
            !IS_EMPTY_SQUARE(g->board[m->from->rank + 2][m->from->file])) {
            return MOVE_INVALID;
        }
        // because the pawn moved by two squares, update the last move
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
        g->lm = create_square(m->from->file, m->from->rank - 1);
        return MOVE_VALID;
    }

    // these are the one square moves for the pawns
    direction = white ? 0b111000000 : 0b000000111;
    return handle_simple_piece(g, m, direction, 1);
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
