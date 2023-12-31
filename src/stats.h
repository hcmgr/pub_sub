//stats.h//
//--------------//
//stats.c abstracts away all functionality to do with psserver's statistics.
//--------------//

#ifndef STATS
#define STATS

#include <semaphore.h>
#include <signal.h>

/* Defines the Stats structure which holds various statistics of psserver 
 * and its clients. It holds the following variables:
 *
 *      clientsCurr - number of clients currently connected
 *      clientsAll - number of clients connected over psserver's lifetime
 *      pub - number of successful pub commands sent to psserver
 *      sub - number of successful sub commands sent to psserver
 *      unsub - number of successful unsub commands sent to psserver
 * */
typedef struct {
    int clientsCurr;
    int clientsAll;
    int pub;
    int sub;
    int unsub;
} Stats;

/* Defines the StatsThreadArgs structure we pass the statistics thread we 
 * spawn (see statistics_thread()). This structure holds the following data:
 *
 *      stats - Stats structure to hold psserver's current statistics
 *      signalMask - set of signals to block (just SIGHUP in our case)
 *      statsLock - lock to access the stats structure
 *
 * */
typedef struct {
    Stats* stats; 
    sigset_t* signalMask;
    sem_t* statsLock;
} StatsThreadArgs;

/* Each of these constants encodes a certain type of stat update one may
 * perform to psserver's statistics.
 *
 * NOTE: we only ever increment all stats EXCEPT for the number of clients
 * currently connected (this is decremented when a client disconnects)
 * */
enum StatChangeCodes {
    INC_CLIENTS_CURR,
    DEC_CLIENTS_CURR,
    INC_CLIENTS_ALL,
    INC_PUB,
    INC_SUB,
    INC_UNSUB
};

/* update_stat
 * -----------
 * This is a general method to update any of psserver's 5 stats it keeps 
 * track of. 
 *
 * stats - pointer to psserver's Stats structure
 * statType - one of the StatChangeCodes (see above) that indicates what
 *            operation we are performing on which stat
 * statsLock - lock on psserver's Stats structure
 *
 * */
void update_stat(Stats* stats, int statType, sem_t* statsLock);

/* stats_init
 * ----------
 * Initialises a Stats structure and returns a pointer to it.
 *
 * NOTE: structure is dynamically allocated and thus must be free()'d
 * */
Stats* stats_init();

/* print_statistics
 * ----------------
 * Prints out the given set of statistics server emits when it received 
 * SIGHUP
 *
 * s - statistics struct 
 * */
void print_statistics(Stats* stats);

/* init_stats_thread_args
 * ----------------------
 * Initialises the StatsThreadArgs structure we pass to the stats thread
 * we spawn. 
 *
 * stats - Stats structure to hold the psserver's current statistics
 * statsLock - lock to access the stats structure
 *
 * Returns:
 *      the newly formed StatsThreadArgs structure
 * */
StatsThreadArgs* init_stats_thread_args(Stats* stats, sem_t* statsLock);

/* statistics_thread
 * -----------------
 * This is a thread that is spawned at the beginning of psserver's run-time.
 * It sits in an infinite loop until SIGHUP is detected, at which point, 
 * psserver's current statistics are printed out.
 *
 * arg - StatsThreadArgs structure 
 *
 * Exits:
 *      when psserver exits
 *
 * */
void* statistics_thread(void* arg);

/* start_statistics_thread
 * -----------------------
 * Spawns the statistics thread (see above).
 *
 * sta - StatsThreadArgs structure we pass the statistics thread
 *
 * */
void start_statistics_thread(StatsThreadArgs* sta);

#endif //STATS



