/**
 * critical_concurrency
 * CS 341 - Fall 2022
 */
#include "barrier.h"

// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    pthread_mutex_destroy(&(barrier->mtx)); ;
    pthread_cond_destroy(&(barrier->cv));
    return 0;
}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    pthread_mutex_init(&barrier->mtx, NULL);
    pthread_cond_init(&barrier->cv, NULL);

    barrier->n_threads = num_threads;
    barrier->count = 0;
    barrier->times_used = 1;
    return 0;
}

int barrier_wait(barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mtx);
    while (barrier->times_used != 1) {
        pthread_cond_wait(&(barrier->cv), &(barrier->mtx));
    }

    barrier->count++;
    if (barrier->count == barrier->n_threads) {
        barrier->times_used = 0;
        --(barrier->count);
        pthread_cond_broadcast(&(barrier->cv));
    } else {
        while (barrier->count != barrier->n_threads && barrier->times_used == 1){
            pthread_cond_wait(&(barrier->cv), &(barrier->mtx));
        }
        --(barrier->count);
        if (barrier->count == 0){
            barrier->times_used = 1;
        }
        pthread_cond_broadcast(&(barrier->cv));
    }
    pthread_mutex_unlock(&(barrier->mtx));
    return 0;
}
