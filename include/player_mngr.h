#ifndef SEMESTRALKA_PLAYER_MNGR_H
#define SEMESTRALKA_PLAYER_MNGR_H
#define MAX_PLAYER_NAME_LENGTH 64

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
    char name[MAX_PLAYER_NAME_LENGTH + 1];
};

/**
 * Attempts to lookup the player in the disconnected list by name
 * @param name the player's name
 * @param p the player to be set
 * @return 0 if found
 */
int lookup_dc_player(char* name, struct player** p);

/**
 * Attempts to lookup the player in the players arr by the file descriptor
 * @param fd the client's file descriptor
 * @param p the player to be set
 * @return 0 if found
 */
int lookup_player_by_fd(int fd, struct player** p);

/**
 * Validates and changes the players name
 * @param p the player
 * @param name the new name
 * @return 0 if everything went ok
 */
int change_player_name(struct player* p, char* name);

/**
 * Attempts to lookup the player in the players arr by the name
 * @param name the player's name
 * @param p the player to be set
 * @return 0 if found
 */
int lookup_player_by_name(char* name, struct player** p);
/**
 * Handles the connection of a client
 * @param fd the client's file descriptor
 * @return 0 if everything went alright
 */
int handle_new_player(int fd);

/**
 * Changes the state of the player. Used for logging purposes only.
 * @param p the player
 * @param ps the state
 */
void change_state(struct player* p, enum player_state ps);

/**
 * Handles the disconnection of the client - storing the player
 * in the disconnected array to be later restored
 * @param p the client's file descriptor
 * @return
 */
int handle_disconnection(struct player* p);

#endif //SEMESTRALKA_PLAYER_MNGR_H
