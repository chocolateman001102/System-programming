/**
 * critical_concurrency
 * CS 341 - Fall 2022
 */
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"

int main(int argc, char **argv) {
    if (argc != 1) {
        printf("usage: %s test_number return_code\n", argv[0]);
        exit(1);
    }
    return 0;
}
