#ifndef SEMESTRALKA_QUEUE_MNGR_H
#define SEMESTRALKA_QUEUE_MNGR_H

void init_qman();

int add_to_queue(int fd);

int remove_from_queue(int fd);

#endif //SEMESTRALKA_QUEUE_MNGR_H
