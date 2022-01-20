#ifndef SEMESTRALKA_QUEUE_MNGR_H
#define SEMESTRALKA_QUEUE_MNGR_H

#include "player_mngr.h"

/**
 * Initializes the queue manager
 */
void init_qman();

/**
 * Adds the player to the queue
 * @param p the player
 * @return 0 if successfully added
 */
int add_to_queue(struct player* p);

/**
 * Removes the player from the queue
 * @param p the player
 * @return 0 if successfully removed
 */
int remove_from_queue(struct player* p);

int qman_handle_dc(struct player* p);

#endif //SEMESTRALKA_QUEUE_MNGR_H
