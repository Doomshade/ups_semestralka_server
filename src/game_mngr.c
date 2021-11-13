#include "../include/game_mngr.h"
#include "../include/server.h"
#include "../include/player_mngr.h"
#include "string.h"
#include <regex.h>
#include <stdio.h>

#define FEN_PATTERN "((([rnbqkpRNBQKP1-8]+)\\/){7}([rnbqkpRNBQKP1-8]+)) ([wb]) (K?Q?k?q?|\\-) (([a-h][0-7])|\\-) (\\d+) (\\d+)"

struct game** games = NULL;
int free_game_index = 0;

void free_game(struct game* g) {
    free(g->board);
    free(g);
}

void setup_game_mngr() {
    // the manager has already been set up
    if (games) {
        return;
    }

    games = calloc(MAX_GAME_COUNT, sizeof(struct game*));
    free_game_index = 0;
}

int add_game(struct game* g) {
    int i;
    if (!g) {
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
    struct game* g;
    if (idx >= MAX_GAME_COUNT) {
        return;
    }

    free_game(games[idx]);
    games[idx] = NULL;
}

void finish_game(struct game* g, int winner) {
    int i;
    if (!g) {
        return;
    }

    

    for (i = 0; i < MAX_GAME_COUNT; ++i) {
        if (games[i] && g == games[i]) {
            remove_game_by_idx(i);
            return;
        }
    }
}

struct game* lookup_game(char* name, struct player** p) {
    if (name == NULL || p == NULL) {
        return NULL;
    }
    for (int i = 0; i < MAX_GAME_COUNT; ++i) {
        struct game* g = games[i];
        if (g == NULL) {
            continue;
        }
        if (strcmp(g->white->name, name) == 0) {
            g->white = *p;
            return g;
        } else if (strcmp(g->black->name, name) == 0) {
            g->black = *p;
            return g;
        }
    }
    return NULL;
}

int setup_game(struct game* g, char* fen) {
    if (fen == NULL) {
        return 1;
    }
    regex_t reg;
    char buf[256];
    char* board[8] = {0};
    char* token;
    char* es;
    int rank = 0;
    int ret;
    int i = 0;

    ret = regcomp(&reg, fen, 0);

    // the regex is invalid (should not happen)
    if (ret != 0) {
        fprintf(stderr, "(%d) Could not compile regex\n", ret);
        return ret;
    }

    ret = regexec(&reg, FEN_PATTERN, 0, NULL, 0);

    // the fen is invalid
    if (ret != 0) {
        fprintf(stderr, "(%d) Invalid FEN pattern: %s\n", ret, fen);
        return ret;
    }

    // the first part is the board
    // "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR" w KQkq - 0 1
    token = strtok(fen, "/");

    // the "row" in chessboard
    while (token != NULL && rank < 8) {
        // check for an empty space
        es = strchr(token, ' ');

        // we reached the end of the first part: "RNBQKBNR w KQkq - 0 1"
        if (es != NULL) {
            memset(buf, 0, 256);
            while (*es != ' ') {
                buf[i++] = *es;
                es++;
            }
            break;
        }
        token = strtok(NULL, "/");
        board[rank] = token;
        rank++;
    }

    // now we check who's to move

    regfree(&reg);
    return 0;
}

struct game* create_game(struct player* white, struct player* black, bool white_to_move) {
    if (white == NULL || black == NULL) {
        return NULL;
    }
    struct game* g = malloc(sizeof(struct game));
    if (g == NULL) {
        return NULL;
    }
    g->white = white;
    g->black = black;
    struct chessboard* board = malloc(sizeof(struct chessboard));
    g->board = board;
    g->white_to_move = white_to_move;
    return g;
}

int game_create(struct player* white, struct player* black) {
    struct game* g;
    if (!white || !black) {
        return 1;
    }
    g = create_game(white, black, true);
    if (!g) {
        return 1;
    }

    return 0;
}

int reconnect_to_game(struct game* g, struct player* p) {

}

void free_player(struct player* p) {
    if (p == NULL) {
        return;
    }
    free(p->name);
    free(p);
}
