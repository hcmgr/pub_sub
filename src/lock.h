//lock.h//
//-------------//
//lock.c abstracts away locking functionality (semaphores)
//-------------//

#ifndef LOCK
#define LOCK

#include <semaphore.h>

/* init_lock
 * ---------
 * Initialises a lock/semaphore with the given lock and increment limit.
 *
 * l - lock
 * numAllowed - increment limit 
 *
 * */
void init_lock(sem_t* l, int numAllowed);

/* take_lock
 * ---------
 * Takes the given lock. If the lock counter is 0, block until it is non-zero. 
 * If the lock is non-zero, decrement the lock. 
 *
 * l - lock to take
 *
 * */
void take_lock(sem_t* l);

/* release_lock
 * ------------
 * Releases the given lock by incrementing the lock counter by 1.
 *
 * l - lock to release
 *
 * */
void release_lock(sem_t* l);

#endif //LOCK

