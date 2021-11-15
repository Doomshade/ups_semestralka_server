#include <stdlib.h>
#include <argp.h>
#include "../include/server.h"

const char* argp_program_version = "Chess server 1.0";
const char* argp_program_bug_address = "<jsmahy@students.zcu.cz>";

static char doc[] = "Semestralni prace UPS 2021.";

// a description of the arguments we accept.
static char args_doc[] = "ARG1";

// the options we understand.
static struct argp_option options[] = {
        {"port", 'p', "10000", 0, "The port"},
        {0}
};

// used by main to communicate with parse_opt.
struct arguments {
    unsigned port;
};

// parse a single option.
static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    // get the input argument from argp_parse, which we
    // know is a pointer to our arguments structure.
    struct arguments* arguments = state->input;

    char* end;
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

// our argp parser
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char** argv) {
    struct arguments arguments;

    // default values
    arguments.port = 10000;

    // parse our arguments; every option seen by parse_opt will
    // be reflected in arguments
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    start_server(arguments.port);
    return 0;
}