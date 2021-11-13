
#ifndef SEMESTRALKA_PLAYER_MNGR_H
#define SEMESTRALKA_PLAYER_MNGR_H

/**
 * The player's state
 */
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

/**
 * The client wrapper
 */
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

/**
 * Attempts to lookup the player in the memory by name
 * @param name the player's name
 * @param p the player to be set
 * @return
 */
int lookup_player_by_name(char* name, struct player** p);

/**
 * Attempts to lookup the player in the memory by the file descriptor
 * @param fd the client's file descriptor
 * @param p the player to be set
 * @return
 */
int lookup_player_by_fd(int fd, struct player** p);

/**
 * Handles the connection of a client
 * @param fd the client's file descriptor
 * @return
 */
int handle_new_connection(int fd);

int handle_possible_reconnection(struct player** p);

/**
 * Handles the disconnection of the client - storing his current state
 * @param fd the client's file descriptor
 * @return
 */
int handle_disconnection(int fd);


#endif //SEMESTRALKA_PLAYER_MNGR_H
