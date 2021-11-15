#include "../include/game_mngr.h"
#include "../include/server.h"
#include "../include/player_mngr.h"
#include "string.h"
#include "../include/packet_registry.h"
#include <regex.h>
#include <stdio.h>
#include <ctype.h>

#define FEN_PATTERN "((([rnbqkpRNBQKP1-8]+)\\/){7}([rnbqkpRNBQKP1-8]+)) ([wb]) (K?Q?k?q?|\\-) (([a-h][0-7])|\\-) (\\d+) (\\d+)"
#define PLAYER_RECON_MESSAGE "Player %s has reconnected to the game"
#define INGAME(g, p) (g->white == p || g->black == p)
#define OPPONENT(g, p) (g->white == p ? g->black : g->white)

struct game** games = NULL;
int free_game_index = 0;

void free_game(struct game* g) {
    free(g->board);
    free(g);
}

void init_gman() {
    // the manager has already been set up
    if (games) {
        return;
    }

    printf("Initializing game manager...\n");
    games = calloc(MAX_GAME_COUNT, sizeof(struct game*));
    free_game_index = 0;
}

int add_game(struct game* g) {
    int i;
    if (!g || !games) {
        return 1;
    }
    if (free_game_index < 0) {
        return 2;
    }

    games[free_game_index] = g;
    free_game_index = -1;
    for (i = 0; i < MAX_GAME_COUNT; ++i) {
        if (!games[i]) {
            free_game_index = i;
            break;
        }
    }
    return 0;
}

void remove_game_by_idx(int idx) {
    if (!games || idx >= MAX_GAME_COUNT) {
        return;
    }

    if (games[idx]) {
        free_game(games[idx]);
        games[idx] = NULL;
    }
}

char* int2bin(int a, char* buffer, int buf_size) {
    buffer += (buf_size - 1);

    for (int i = 31; i >= 0; i--) {
        *buffer-- = (a & 1) + '0';

        a >>= 1;
    }

    return buffer;
}


int finish_game(struct game* g, int winner) {
    int i;
    const int flags = WHITE_WINNER |
                      BLACK_WINNER |
                      WIN_BY_MATE |
                      WIN_BY_RESIGNATION |
                      WIN_BY_TIME;
    char msg[flags + 1];
    struct packet* pckt;
    int ret;

    if (!g || !games) {
        return 1;
    }
    msg[flags] = '\0';
    int2bin(winner, msg, flags);

    for (i = 0; i < MAX_GAME_COUNT; ++i) {
        if (!games[i] || g != games[i]) {
            continue;
        }

        pckt = create_packet(GAME_FINISH_OUT, strlen(msg), msg);
        if (!pckt) {
            return -1;
        }
        if ((ret = send_packet(g->white, pckt)) ||
            (ret = send_packet(g->black, pckt))) {
            return ret;
        }
        remove_game_by_idx(i);
        return 0;
    }
    return 1;
}

struct game* lookup_game(struct player* p) {
    if (!p || !games) {
        return NULL;
    }
    for (int i = 0; i < MAX_GAME_COUNT; ++i) {
        struct game* g = games[i];
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
    if (!white || !black || !games) {
        return NULL;
    }
    struct game* g = malloc(sizeof(struct game));
    if (!g) {
        return NULL;
    }
    g->white = white;
    g->black = black;

    g->board = malloc(sizeof(struct chessboard));
    g->white_to_move = white_to_move;
    return g;
}

int game_create(struct player* white, struct player* black) {
    struct game* g;
    char* fen;
    if (!white || !black || !games) {
        return 1;
    }

    g = create_game(white, black, true);
    if (!g) {
        return 1;
    }
    printf("Creating a new game for %s (W) and %s (B)\n", white->name, black->name);
    change_state(white, PLAY);
    change_state(black, PLAY);

    fen = malloc(sizeof(char) * (strlen(START_FEN) + 1));
    strcpy(fen, START_FEN);
    setup_game(g, fen);

    return add_game(g);
}

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

    if (!g || !g->board) {
        return;
    }

    printf("\n");

    print_files();
    print_hline();
    for (i = 7; i >= 0; --i) {
        printf("%d | ", (i + 1));
        for (j = 0; j < 8; ++j) {
            printf("%c | ", g->board->board[i][j]);
        }
        printf("%d\n", (i + 1));
        print_hline();
    }
    print_files();
}

int setup_game(struct game* g, char* fen) {

    regex_t reg;
    char buf[BUFSIZ];
    char* board[8] = {0};
    char* token;
    char* es;
    int rank = 0;
    int ret;
    int i = 0, j = 0;
    regmatch_t match;

    if (!g || !fen || !games) {
        return 1;
    }
    // TODO fix the regex
    /*ret = regcomp(&reg, FEN_PATTERN, 0);

    // the regex is invalid (should not happen)
    if (ret) {
        fprintf(stderr, "(%d) Could not compile regex\n", ret);
        return ret;
    }

    ret = regexec(&reg, fen, 0, &match, 0);

    // the fen is invalid
    if (ret == REG_NOMATCH) {
        fprintf(stderr, "(%d) Invalid FEN pattern: %s\n", ret, fen);
        return ret;
    }*/

    // the first part is the board
    // "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR" w KQkq - 0 1
    token = strtok(fen, "/");

    // the "row" in chessboard
    while (token != NULL && rank < 8) {
        // check for an empty space
        es = strchr(token, ' ');

        // we reached the end of the first part: "RNBQKBNR w KQkq - 0 1"
        if (es != NULL) {
            memset(buf, 0, BUFSIZ);

            // shift until we find non-space char
            while (*es != ' ') {
                buf[i++] = *es;
                es++;
            }
            board[rank] = buf;
            break;
        }
        token = strtok(NULL, "/");
        board[rank++] = token;
    }

    for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j) {
            g->board->board[i][j] = ' ';
        }
    }

    // TODO
    g->board->board[0][0] = 'r';
    g->board->board[0][1] = 'n';
    g->board->board[0][2] = 'b';
    g->board->board[0][3] = 'q';
    g->board->board[0][4] = 'd';
    g->board->board[0][5] = 'b';
    g->board->board[0][6] = 'n';
    g->board->board[0][7] = 'r';

    for (i = 0; i < 8; ++i) {
        g->board->board[1][i] = 'p';
        g->board->board[6][i] = 'P';
        g->board->board[7][i] = (char) toupper(g->board->board[0][i]);
    }
    // now we check who's to move
    // "w KQkq - 0 1"
    // TODO
    print_board(g);

    regfree(&reg);
    return 0;
}

char* generate_fen(struct chessboard* board) {
    // TODO
    return START_FEN;
}

int reconnect_to_game(struct player* pl, struct game* g) {
    struct player* op;
    struct packet* pc;
    int ret;
    char* fen;
    char buf[BUFSIZ];

    if (!INGAME(g, pl)) {
        printf("Player %s is not in game: %s vs %s\n", pl->name, g->white->name, g->black->name);
        return 1;
    }

    // send the reconnecting player the game state
    fen = generate_fen(g->board);
    pc = create_packet(GAME_START_OUT, strlen(fen), fen);
    printf("Sending %s game start packet...\n", pl->name);
    ret = send_packet(pl, pc);
    free_packet(pc);

    // send the information to the opponent about the
    // reconnection of the disconnected player
    // TODO create a new packet ID for this
    op = OPPONENT(g, pl);
    sprintf(buf, PLAYER_RECON_MESSAGE, op->name);
    pc = create_packet(MESSAGE_OUT, strlen(buf), buf);
    printf("Sending %s that %s has reconnected...\n", op->name, pl->name);
    ret = send_packet(op, pc);
    free_packet(pc);
    return ret;
}

bool is_players_piece(bool white, char piece) {
    char target_piece = (char) (white ? tolower(piece) : toupper(piece));
    return piece == target_piece;
}

int move_piece(struct game* g, struct player* p, unsigned int rank_from, unsigned int file_from, unsigned int rank_to,
               unsigned int file_to) {
    char pce_from;
    bool white;
    if (!g || !p || (file_from | rank_from | file_to | file_from) >= 8) {
        return 1;
    }
    white = g->white == p;

    // the player is not on the move
    // ignore the packet
    if (white != g->white_to_move) {
        printf("It is not %s's turn yet!\n", p->name);
        return 1;
    }
    pce_from = g->board->board[rank_from][file_from];

    if (pce_from == ' ') {
        printf("No piece found on [%d, %d]\n", rank_from, file_from);
        return 1;
    }
    if (!is_players_piece(white, pce_from)) {
        printf("Piece on [%d, %d] (%d) is the opponent's piece!\n", rank_from, file_from, pce_from);
        return 1;
    }

    printf("[%d,%d] '%c' -> [%d,%d] '%c'\n", rank_from, file_from, pce_from, rank_to, file_to,
           g->board->board[rank_to][file_to]);
    g->board->board[rank_to][file_to] = pce_from;
    g->board->board[rank_from][file_from] = ' ';
    g->white_to_move = !g->white_to_move;
    print_board(g);
    return 0;
}
