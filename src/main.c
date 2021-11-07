#include <stdlib.h>
#include <stdio.h>
#include <argp.h>
#include <stdbool.h>
#include "../include/packet_handler.h"

/*int IDwithBuiltinKTH(problemposition) {
    for (int depth = 1; depth < maxDepth; depth += depthstep) {
        r = maxturn(problemposition, depth, null);
        if r = PROVED return r;

    }
    return UNSOLVED;
}

int maxturn(position, depth, killerposition) {
    if position is terminal, store its value and return the value;
    retrieve position from the transposition table;
    if solved, return its value;
    if depth = 0, store UNSOLVED and return it;
    if killerposition != null{
        retrieve killerposition from the transposition table and get the best move as km;
        if km is also legal in position {
            make move km in position;
            make move km in killerposition;
            r = minturn(position, depth - 1, killerposition);
            unmake move km in position;
            unmake move km in killerposition;
            if r = PROVED, store r and return it;
        }
        killerposition = null;
    }
    generate legal moves in position: m[0], ..., m[nm - 1];
    for each move m[i]{
        if m[i] = km, continue;
        make move m[i] in position;
        r = minturn(position, depth - 1, killerposition);
        unmake move m[i] in position;
        if r = PROVED, store r and return it;
    }
    store UNSOLVED and return it;
}

int minturn(position, depth, killerposition) {
    if position is terminal, store its value and return the value;
    retrieve position from transposition table;
    if solved, return its value;
    if depth = 0, store UNSOLVED and return it;
    generate the legal moves in position m[0], ..., m[nm - 1];
    killerroot = null;
    for each move m[i] {
        make move m[i] in position;
        if killerposition != null {
            if m[i] is also legal in killerposition, make move m[i] in killerposition;
            else killerposition = null;
        }
        if killerposition != null {
            r2 = maxturn(position, depth - 1, killerposition);
        } else {
            r2 = maxturn(position, depth - 1, killerroot);
        }
        if killerposition = null and killerroot = null and r2 = PROVED {
            kilerroot = position;
        }
        unmake move m[i] in position;
        if killerposition != null, unmake move m[i] in killerposition;
        if r2 != PROVED, store UNSOLVED and return it;
    }
    store PROVED and return it;
}*/

const char* argp_program_version = "Chess server 1.0";
const char* argp_program_bug_address = "<jsmahy@students.zcu.cz>";

static char doc[] = "Semestralni prace UPS 2021.";
/* A description of the arguments we accept. */
static char args_doc[] = "ARG1";

/* The options we understand. */
static struct argp_option options[] = {
        {"port", 'p', "10000", 0, "The port"},
        {0}
};

/* Used by main to communicate with parse_opt. */
struct arguments {
    long port;
};

/* Parse a single option. */
static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    /* Get the input argument from argp_parse, which we
       know is a pointer to our arguments structure. */
    struct arguments* arguments = state->input;

    char* end;
    printf("Key and arg: %c (0x%x) %s\n", key, key, arg);
    switch (key) {
        case 'p':
            arguments->port = strtol(arg, &end, 10);
        case ARGP_KEY_ARG:
        case ARGP_KEY_END:
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char** argv) {
    struct arguments arguments;

    /* Default values. */
    arguments.port = 10000;

    /* Parse our arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    printf("port: %ld\n", arguments.port);

    exit(0);
}