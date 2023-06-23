//lock.c//
//-------------//
//lock.c abstracts away locking functionality (semaphores)
//-------------//

#include "lock.h"

void init_lock(sem_t* l, int numAllowed) {
    sem_init(l, 0, numAllowed);
}

void take_lock(sem_t* l) {
    sem_wait(l);
}

void release_lock(sem_t* l) {
    sem_post(l);
}

