/**
 * teaching_threads
 * CS 341 - Fall 2022
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct each_reduce {
    int *list;
    size_t list_len;
    reducer reduce_func;
    int base_case;
} each_reduce;


void *start_rt(void *data) {
    each_reduce *temp = (each_reduce*)data;
    int *result = malloc(sizeof(int));
    *result = reduce(temp->list, temp->list_len, temp->reduce_func, temp->base_case);
    return (void*) result;
}

/* You should create a start routine for your threads. */

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case, size_t num_threads) {
    each_reduce *reduc = malloc(num_threads * sizeof(each_reduce));
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

    int *int_list_main = malloc(num_threads * sizeof(int));

    size_t *sizes = malloc(sizeof(size_t*) * num_threads);

    size_t base = list_len / num_threads;
    size_t left = list_len % num_threads;

    for(size_t i = 0; i< num_threads;++i) {
        sizes[i] = base;
        if (i < left)
            sizes[i]++;
    }

    size_t offset = 0;
    size_t running_threads = 0;
    for (size_t i = 0; i < num_threads; ++i) {
        //puts("running for");
        if (sizes[i] > 0) {
            reduc[i] = (each_reduce){list + offset, sizes[i], reduce_func, base_case};

            pthread_create(threads + i, NULL, start_rt, reduc + i);

            offset += sizes[i];
            ++running_threads;
        }
    }
    for (size_t i = 0; i < running_threads; ++i) {
        //puts("putting thread n");
        void *each_ele = NULL;
        pthread_join(threads[i], &each_ele);
        int_list_main[i] = *((int *)each_ele);
        free(each_ele);
    }
    int result = reduce(int_list_main, running_threads, reduce_func, base_case);

    free(reduc);
    free(threads);
    free(int_list_main);

    free(sizes);
    return result;
}

