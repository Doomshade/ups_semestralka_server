
#ifndef SEMESTRALKA_PLAYER_MNGR_H
#define SEMESTRALKA_PLAYER_MNGR_H

enum player_state {

    /**
     * The player has disconnected
     */
    DISCONNECTED,

    /**
     * The initial state -- when the player connects to the server
     */
    JUST_CONNECTED,

    /**
     * The player is in a queue
     */
    QUEUE,

    /**
     * The player is in a game and white is to move
     */
    WHITE_TO_MOVE,

    /**
     * The player is in a game and black is to move
     */
    BLACK_TO_MOVE
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

#endif //SEMESTRALKA_PLAYER_MNGR_H
