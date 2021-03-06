#include "../include/game_mngr.h"
#include <string.h>
#include "../include/packet_registry.h"
#include "../include/chesspiece.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define FEN_PATTERN "((([rnbqkpRNBQKP1-8]+)\\/){7}([rnbqkpRNBQKP1-8]+)) ([wb]) (K?Q?k?q?|\\-) (([a-h][0-7])|\\-) (\\d+) (\\d+)"
#define PLAYER_RECON_MESSAGE "Player %s has reconnected to the game"
#define PLAYER_DISCON_MESSAGE "Player %s has disconnected"

#define MOVE(board, m, piece) \
PIECE_SQ((board), (m)->from) = EMPTY_SQUARE;\
PIECE_SQ((board), (m)->to) = (piece);

struct matches {
    struct game** games;
    int free_game_index;
    unsigned max_game_count;
};

static struct matches* matches = NULL;

int get_king_sq(struct game* g, bool white, struct square* sq_ptr);

void print_hline() {
    int i;
    printf("  -");
    for (i = 0; i < 8; ++i) {
        printf("----");
    }
    printf("\n");
}

void print_files() {
    int i;

    printf("    ");
    for (i = 'A'; i < 'A' + 8; ++i) {
        printf("%c   ", (char) i);
    }
    printf("\n");
}

void print_board(struct game* g) {
    int i, j;

    if (!g) {
        return;
    }

    printf("\n");

    print_files();
    print_hline();
    for (i = 7; i >= 0; --i) {
        printf("%d | ", (i + 1));
        for (j = 0; j < 8; ++j) {
            printf("%c | ", g->board[i][j]);
        }
        printf("%d\n", (i + 1));
        print_hline();
    }
    print_files();
}

void free_game(struct game** g) {
    free(*g);
    *g = NULL;
}

void init_gman(unsigned player_count) {
    // the manager has already been set up
    if (matches) {
        return;
    }

    printf("Initializing game manager...\n");
    matches = malloc(sizeof(struct matches));
    matches->free_game_index = 0;
    matches->max_game_count = player_count / 2;
    matches->games = calloc(matches->max_game_count, sizeof(struct game*));
}

int add_game(struct game* g) {
    int i;
    if (!g || !matches) {
        return 1;
    }
    if (matches->free_game_index < 0) {
        return 2;
    }

    matches->games[matches->free_game_index] = g;
    matches->free_game_index = -1;
    for (i = 0; i < matches->max_game_count; ++i) {
        if (!matches->games[i]) {
            matches->free_game_index = i;
            break;
        }
    }
    return 0;
}

void remove_game_by_idx(int idx) {
    if (!matches || idx >= matches->max_game_count) {
        return;
    }

    if (matches->games[idx]) {
        free_game(&(matches->games[idx]));
        matches->games[idx] = NULL;
    }
}


int finish_game(struct game* g, int winner) {
    int i;
    char msg[3 + 1] = {0};
    int ret;

    if (!g || !matches) {
        return 1;
    }
    sprintf(msg, "%03d", winner);

    // look for the game
    for (i = 0; i < matches->max_game_count; ++i) {
        // that's not the game we are looking for, continue
        if (!matches->games[i] || g != matches->games[i]) {
            continue;
        }


        // game found, send the packet
        ret = send_packet(g->white, GAME_FINISH_OUT, msg);
        ret = send_packet(g->black, GAME_FINISH_OUT, msg);

        // change the player state and remove it from memory
        change_state(g->white, LOGGED_IN);
        change_state(g->black, LOGGED_IN);
        remove_game_by_idx(i);

        return ret;
    }
    return 1;
}

struct game* lookup_game(struct player* p) {
    if (!p || !matches) {
        return NULL;
    }
    for (int i = 0; i < matches->max_game_count; ++i) {
        struct game* g = matches->games[i];
        if (g == NULL) {
            continue;
        }
        if (INGAME(g, p)) {
            return g;
        }
    }
    return NULL;
}

struct game* create_game(struct player* white, struct player* black, bool white_to_move) {
    struct game* g;
    if (!white || !black || !matches) {
        return NULL;
    }
    g = malloc(sizeof(struct game));
    if (!g) {
        return NULL;
    }
    memset(g, 0, sizeof(struct game));

    g->white = white;
    g->black = black;
    g->castles = CASTLES_BLACK_QUEENSIDE |
                 CASTLES_BLACK_KINGSIDE |
                 CASTLES_WHITE_QUEENSIDE |
                 CASTLES_WHITE_KINGSIDE;
    g->fullmove_count = 1;
    g->halfmove_clock = 0;
    g->white_to_move = white_to_move;
    g->lm = malloc(sizeof(struct square));
    g->lm->rank = -1;
    g->lm->file = -1;
    return g;
}

int game_create(struct player* white, struct player* black) {
    struct game* g;
    if (!white || !black || !matches->games) {
        return 1;
    }

    g = create_game(white, black, true);
    if (!g) {
        return 1;
    }
    printf("Creating a new game for %s (W) and %s (B)\n", white->name, black->name);
    change_state(white, PLAY);
    change_state(black, PLAY);

    setup_game(g);

    return add_game(g);
}


int setup_game(struct game* g) {
    int i, j;

    if (!g || !matches->games) {
        return 1;
    }

    for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j) {
            g->board[i][j] = ' ';
        }
    }

    g->board[0][0] = ROOK;
    g->board[0][1] = KNIGHT;
    g->board[0][2] = BISHOP;
    g->board[0][3] = QUEEN;
    g->board[0][4] = KING;
    g->board[0][5] = BISHOP;
    g->board[0][6] = KNIGHT;
    g->board[0][7] = ROOK;

    for (i = 0; i < 8; ++i) {
        g->board[1][i] = PAWN;
        g->board[6][i] = TO_BLACK(PAWN);
        g->board[7][i] = TO_BLACK(g->board[0][i]);
    }
    print_board(g);
    return 0;
}

char* generate_fen(struct game* g) {
    int rank; // the rank on the board
    int file; // the file on the board
    int empty; // an empty file counter

    char buf[BUFSIZ] = {0}; // a buffer for the fen
    char* fen; // the fen string that is later returned
    char square[2] = {0}; // a buffer for a single square
    char lm[3] = {0}; // a buffer for last move (for en passant)

    if (!g) {
        return NULL;
    }

    // the first part is the board, starting with black from top
    for (rank = 7; rank >= 0; --rank) {
        for (empty = 0, file = 0; file < 8; ++file) {
            square[0] = g->board[rank][file];
            square[1] = '\0';
            if (IS_EMPTY_SQUARE(square[0])) {
                empty++;
                continue;
            }

            if (empty != 0) {
                // append the num of empty squares to the buf
                sprintf(buf, "%s%d", buf, empty);
            }
            strcat(buf, square);
            empty = 0;
        }
        if (empty != 0) {
            // append the num of empty squares to the buf
            sprintf(buf, "%s%d", buf, empty);
        }
        // don't add the / at the end
        if (rank != 0) {
            strcat(buf, "/");
        }
    }
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR

    strcat(buf, " ");
    strcat(buf, g->white_to_move ? "w" : "b");
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w

    // TODO add castles check
    strcat(buf, " ");
    if (g->castles == 0) {
        strcat(buf, "-");
    } else {
        if (g->castles & CASTLES_WHITE_KINGSIDE) {
            strcat(buf, "K");
        }
        if (g->castles & CASTLES_WHITE_QUEENSIDE) {
            strcat(buf, "Q");
        }
        if (g->castles & CASTLES_BLACK_KINGSIDE) {
            strcat(buf, "k");
        }
        if (g->castles & CASTLES_BLACK_QUEENSIDE) {
            strcat(buf, "q");
        }
    }
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq

    strcat(buf, " ");

    // the last move is not set
    if (!g->lm || (g->lm->file | 7) > 8 || (g->lm->rank | 7) > 8) {
        strcat(buf, "-");
    } else {
        lm[0] = UINT_TO_FILE(g->lm->file);
        lm[1] = UINT_TO_RANK(g->lm->rank);
        strcat(buf, lm);
    }
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -

    sprintf(buf, "%s %d %d", buf, g->halfmove_clock, g->fullmove_count);
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

    fen = malloc(sizeof(char) * (strlen(buf) + 1));
    strcpy(fen, buf);
    return fen;
}

int reconnect_to_game(struct player* pl, struct game* g) {
    struct player* op;
    int ret;
    char* fen;
    char* start_message;
    char buf[BUFSIZ];

    if (!INGAME(g, pl)) {
        printf("Player %s is not in game: %s vs %s\n", pl->name, g->white->name, g->black->name);
        return 1;
    }

    // send the reconnecting player the game state
    fen = generate_fen(g);
    start_message = malloc(strlen(fen) + 2);
    strcpy(start_message, g->white == pl ? "1" : "0");
    strcat(start_message, fen);
    free(fen);
    printf("Sending %s game start packet...\n", pl->name);
    op = OPPONENT(g, pl);
    if (send_packet(pl, RECONNECT_OUT, "")
        || send_packet(pl, GAME_START_OUT, start_message)
        || send_packet(pl, OPPONENT_NAME_OUT, op->name)) {
        printf("Could not reconnect the player!\n");
        return 1;
    }

    // send the information to the opponent about the
    // reconnection of the disconnected player
    // TODO create a new packet ID for this
    sprintf(buf, PLAYER_RECON_MESSAGE, pl->name);

    printf("Sending %s that %s has reconnected...\n", op->name, pl->name);
    ret = send_packet(op, MESSAGE_OUT, buf);
    if (ret == PACKET_RESP_ERR_NOT_RECVD) {
        printf("Could not send the packet because the enemy player is disconnected.\n");
    }
    return ret;
}

int gman_handle_dc(struct player* p) {
    struct game* g;
    struct player* op;
    char buf[BUFSIZ];

    if (!p) {
        return 1;
    }
    VALIDATE_FD(p->fd, 1, matches->max_game_count)
    g = lookup_game(p);
    if (!g) {
        return 0;
    }
    op = OPPONENT(g, p);
    sprintf(buf, PLAYER_DISCON_MESSAGE, p->name);

    if (send_packet(op, OPPONENT_DISCONNECT_OUT, "") || send_packet(op, MESSAGE_OUT, buf)) {
        printf("Failed to send a disconnect packet...\n");
    }
    return 0;
}

bool is_players_piece(bool white, char piece) {
    char target_piece = (char) (white ? toupper(piece) : tolower(piece));
    return piece == target_piece;
}

bool is_in_check(struct game* g, bool white, struct square king_sq);

bool is_mate(struct game* g, bool white, struct square king_sq);

int move_piece(struct game* g, struct player* p, struct move* m) {
    char pce_from;
    char pce_to;
    bool white;
    int ret = 0;
    struct square* lm;
    unsigned int rank_from, file_from, rank_to, file_to;
    struct square pl_king;
    struct square op_king;
    struct square* sq;
    int prev_castles;

    if (!g || !p || !m) {
        return MOVE_INVALID;
    }
    rank_from = m->from->rank;
    file_from = m->from->file;
    rank_to = m->to->rank;
    file_to = m->to->file;

    if ((rank_from | file_from | rank_to | file_to) >= 8) {
        return MOVE_INVALID;
    }
    prev_castles = g->castles;
    white = g->white == p;

    // the player is not on the move
    // ignore the packet
    if (white != g->white_to_move) {
        printf("It is not %s's turn yet!\n", p->name);
        return MOVE_INVALID;
    }

    pce_from = PIECE_SQ(g->board, m->from);
    pce_to = PIECE_SQ(g->board, m->to);

    if (pce_from == ' ') {
        printf("No piece found on [%d, %d]\n", rank_from, file_from);
        return MOVE_INVALID;
    }
    if (!is_players_piece(white, pce_from)) {
        printf("Piece on [%d, %d] (%d) is the opponent's piece!\n", rank_from, file_from, pce_from);
        return MOVE_INVALID;
    }

    printf("[%d,%d] '%c' -> [%d,%d] '%c'\n", rank_from, file_from, pce_from, rank_to, file_to,
           PIECE_SQ(g->board, m->to));

    ret = move(pce_from, g, m);

    // either it's an invalid move -> return 1
    // or it's a valid move -> move the piece
    if (ret == MOVE_INVALID) {
        printf("Invalid move (%c%c-%c%c) for piece %c\n",
               UINT_TO_FILE(m->from->file),
               UINT_TO_RANK(m->from->rank),
               UINT_TO_FILE(m->to->file),
               UINT_TO_RANK(m->to->rank),
               pce_from);
        return ret;
    }

    // the move is valid, we can move it now
    MOVE(g->board, m, pce_from)

    if (get_king_sq(g, white, &pl_king)) {
        fprintf(stderr, "Could not find the king?!\n");
        return MOVE_INVALID;
    }

    if (get_king_sq(g, !white, &op_king)) {
        fprintf(stderr, "Could not find the king?!\n");
        return MOVE_INVALID;
    }

    // now we need to check if the king is in check
    if (is_in_check(g, white, pl_king)) {
        printf("The king is in check, cannot make that move!\n");

        // unmake the move
        PIECE_SQ(g->board, m->from) = pce_from;
        PIECE_SQ(g->board, m->to) = pce_to;
        return MOVE_INVALID;
    }

    if (ret & MOVE_CASTLES) {
        // move the rook to its place
        PIECE(g->board, m->from->rank, ret & MOVE_LONG_CASTLES ? 3 : 5) = PIECE(g->board, m->from->rank,
                                                                                ret & MOVE_LONG_CASTLES ? 0 : 7);
        PIECE(g->board, m->from->rank, ret & MOVE_LONG_CASTLES ? 0 : 7) = EMPTY_SQUARE;
    } else if (ret == MOVE_EN_PASSANT) {
        lm = g->lm;
        if (IS_WHITE(PIECE_SQ(g->board, m->from))) {
            lm->rank--;
        } else {
            lm->rank++;
        }
        PIECE_SQ(g->board, lm) = EMPTY_SQUARE;
    }

    if (is_in_check(g, !white, op_king)) {
        printf("CHECK DETECTED\n");
    }

    if (is_mate(g, !white, op_king)) {
        printf("MATE DETECTED!!\n");
        return MOVE_CHECKMATE;
    }

    // we are taking a piece or moving a pawn, reset the halfmove
    if (!IS_EMPTY_SQUARE(PIECE_SQ(g->board, m->to)) || IS_PAWN(PIECE_SQ(g->board, m->from))) {
        g->halfmove_clock = 0;
    } else {
        g->halfmove_clock++;
    }

    // increment the fullmove count after black's move
    if (!g->white_to_move) {
        g->fullmove_count++;
    }
    g->white_to_move = !g->white_to_move;
#ifdef SERVER_DEBUG_MODE
    //strcat(buf, "\n");
    //print_board(g, buf);
    //send_packet(p, MESSAGE_OUT, buf);
    //send_packet(OPPONENT(g, p), MESSAGE_OUT, buf);
#else
    print_board(g);
#endif
    return ret;
}

int get_king_sq(struct game* g, bool white, struct square* sq_ptr) {
    unsigned rank;
    unsigned file;
    char piece;
    // iterate through the board and find the king

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            piece = PIECE(g->board, rank, file);

            if (IS_KING(piece) && IS_WHITE(piece) == white) {
                *sq_ptr = (struct square) {.rank = rank, .file = file};
                return 0;
            }
        }
    }
    return 1;
}

// TODO this is kind of a copy of is_in_check
bool is_mate(struct game* g, bool white, struct square king_sq) {
    int move_resp;
    unsigned rank;
    unsigned file;
    unsigned rank_to;
    unsigned file_to;
    char piece;
    char prev_piece;
    struct move m;
    struct move flip_m;
    bool is_not_in_check;
    struct square* lm;
    int prev_castles;

    if (!is_in_check(g, white, king_sq)) {
        return false;
    }
    prev_castles = g->castles;

    // iterate through every piece
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            piece = PIECE(g->board, rank, file);

            // the piece is empty or opponents
            if (piece == EMPTY_SQUARE || white != IS_WHITE(piece)) {
                continue;
            }
            // the piece's position
            m.from = &((struct square) {.rank = rank, .file = file});

            // iterate through every square
            for (rank_to = 0; rank_to < 8; ++rank_to) {
                for (file_to = 0; file_to < 8; ++file_to) {
                    if (rank_to == rank && file_to == file) {
                        continue;
                    }
                    m.to = &((struct square) {.rank = rank_to, .file = file_to});

                    // store the last move before moving the piece as it gets modified
                    lm = g->lm;
                    move_resp = move(piece, g, &m);
                    g->lm = lm;
                    if (move_resp != MOVE_VALID && move_resp != MOVE_EN_PASSANT) {
                        continue;
                    }
                    // save the piece
                    prev_piece = PIECE_SQ(g->board, m.to);
                    // make the move
                    MOVE(g->board, &m, piece);

                    if (get_king_sq(g, white, &king_sq)) {
                        fprintf(stderr, "wtf\n");
                        return true;
                    }

                    // save the state
                    is_not_in_check = !is_in_check(g, white, king_sq);

                    // unmake the move
                    flip_m.from = m.to;
                    flip_m.to = m.from;
                    MOVE(g->board, &flip_m, piece);

                    // set the piece back
                    PIECE_SQ(g->board, m.to) = prev_piece;

                    // see if the king is not in check
                    if (is_not_in_check) {
                        g->castles = prev_castles;
                        printf("Piece %c can move to %c%c to avoid mate\n", piece, UINT_TO_FILE(file_to),
                               UINT_TO_RANK(rank_to));
                        return false;
                    }

                }
            }
        }
    }
    g->castles = prev_castles;
    printf("No piece can avoid mate for %s, RIP!\n", white ? "White" : "Black");
    return true;
}

bool is_in_check(struct game* g, bool white, struct square king_sq) {
    struct move m;
    unsigned rank;
    unsigned file;
    char piece;
    int prev_castles;

    // iterate through the board and find the king

    //printf("%s's king square: %c%c\n", white ? "White" : "Black", UINT_TO_FILE(king_sq.file),
    //      UINT_TO_RANK(king_sq.rank));
    // now check if the enemy pieces are attacking this square
    prev_castles = g->castles;
    m.to = &king_sq;
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            piece = PIECE(g->board, rank, file);

            // there's no piece or the piece is a friendly one
            if (piece == EMPTY_SQUARE || white == IS_WHITE(piece)) {
                continue;
            }
            m.from = &((struct square) {.rank = rank, .file = file});

            // an opponents piece found, check if its moves contain the king's square
            if (move(piece, g, &m) != MOVE_INVALID) {
                g->castles = prev_castles;
                // the move was not invalid, return true that the king is indeed in check after the given move
                return true;
            }
        }
    }
    g->castles = prev_castles;

    return false;
}

struct move* create_move(struct square* from, struct square* to) {
    struct move* m;

    if (!from || !to) {
        return NULL;
    }

    m = malloc(sizeof(struct move));
    if (!m) {
        return NULL;
    }
    m->from = from;
    m->to = to;
    return m;
}

void free_move(struct move** m) {
    free((*m)->from);
    free((*m)->to);
    free((*m));
    *m = NULL;
}

struct square* create_square(unsigned int file, unsigned int rank) {
    struct square* s;

    if ((file | rank) >= 8) {
        return NULL;
    }

    s = malloc(sizeof(struct square));
    if (!s) {
        return NULL;
    }
    s->file = file;
    s->rank = rank;
    return s;
}
