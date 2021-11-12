
#ifndef SEMESTRALKA_PLAYER_MNGR_H
#define SEMESTRALKA_PLAYER_MNGR_H

enum player_state {

    /**
     * The initial state -- when the player connects to the server
     */
    JUST_CONNECTED,

    /**
     * The player has logged in (he has sent his name)
     */
    LOGGED_IN,

    /**
     * The player is in a queue
     */
    QUEUE,

    /**
     * The player is in a game
     */
    PLAY
};

struct player {

    /**
     * The file descriptor of this player
     */
    int fd;

    /**
     * The current state of the player
     */
    enum player_state ps;

    /**
     * The name of this player
     */
    char* name;
};

int lookup_player_by_name(char* name, struct player** p);

int lookup_player_by_fd(int fd, struct player** p);

int handle_new_connection(int fd);

int handle_possible_reconnection(struct player** p);

int handle_disconnection(int fd);


#endif //SEMESTRALKA_PLAYER_MNGR_H
