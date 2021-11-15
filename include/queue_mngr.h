#ifndef SEMESTRALKA_QUEUE_MNGR_H
#define SEMESTRALKA_QUEUE_MNGR_H

#include "player_mngr.h"

void init_qman();

int add_to_queue(struct player* p);

int remove_from_queue(struct player* p);

#endif //SEMESTRALKA_QUEUE_MNGR_H
