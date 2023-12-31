//stats.c//
//--------------//
//This file abstracts away all functionality to do with psserver's statistics.
//--------------//

#include "stats.h"
#include "lock.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>

#define SUCCESS 0

void update_stat(Stats* stats, int statType, sem_t* statsLock) {
    take_lock(statsLock);
    switch (statType) {
        case INC_CLIENTS_CURR:
            stats->clientsCurr++;
            break;
        case DEC_CLIENTS_CURR:
            stats->clientsCurr--;
            break;
        case INC_CLIENTS_ALL:
            stats->clientsAll++;
            break;
        case INC_PUB:
            stats->pub++;
            break;
        case INC_SUB:
            stats->sub++;
            break;
        case INC_UNSUB:
            stats->unsub++;
            break;
    }
    release_lock(statsLock);
}

Stats* stats_init() {
    Stats* stats = malloc(sizeof(Stats));
    memset(stats, 0, sizeof(Stats));
    return stats;
}

void print_statistics(Stats* stats) {
    fprintf(stderr, "Connected clients:%d\n", stats->clientsCurr);
    fprintf(stderr, "Completed clients:%d\n", stats->clientsAll);
    fprintf(stderr, "pub operations:%d\n", stats->pub);
    fprintf(stderr, "sub operations:%d\n", stats->sub);
    fprintf(stderr, "unsub operations:%d\n", stats->unsub);
}

StatsThreadArgs* init_stats_thread_args(Stats* stats, sem_t* statsLock) {
    //initialise struct itself
    StatsThreadArgs* sta = malloc(sizeof(StatsThreadArgs));  
    memset(sta, 0, sizeof(StatsThreadArgs));
    sta->stats = stats;
    sta->statsLock = statsLock;

    //REFERENCE:
    //  the following 7 lines of code are based off the example provided 
    //  in the pthread_sigmask man page on moss

    //initialise the signal mask
    sigset_t* mask = malloc(sizeof(sigset_t));
    sigemptyset(mask);
    sigaddset(mask, SIGHUP);
    int result = pthread_sigmask(SIG_BLOCK, mask, NULL); 
    if (result != SUCCESS) {
        //handle error
    }
    sta->signalMask = mask;
    return sta;
}

void* statistics_thread(void* arg) {
    StatsThreadArgs* sta = (StatsThreadArgs*)arg;
    int signal;
    while(true) {
        int result = sigwait(sta->signalMask, &signal);
        if (result != SUCCESS) {
            //handle error
        }
        take_lock(sta->statsLock);
        print_statistics(sta->stats);
        release_lock(sta->statsLock);
    }
}

void start_statistics_thread(StatsThreadArgs* sta) {
    pthread_t threadId;
    pthread_create(&threadId, NULL, statistics_thread, sta);
    pthread_detach(threadId);
}


