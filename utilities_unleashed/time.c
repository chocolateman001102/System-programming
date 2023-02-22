/**
 * utilities_unleashed
 * CS 341 - Fall 2022
 */
#include "format.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(int argc, char *argv[]) {
    //printf("%d %s %s %s",argc,argv[0],argv[1],argv[2]);
    if (argc < 2) {
        print_time_usage();
    }
    pid_t child_id = fork();
    if (child_id < 0) {
        print_fork_failed();
    } else if(child_id == 0) { 
        execvp(argv[1], argv+1);
        print_exec_failed();
    }
    struct timespec start, end;
    int status;

    clock_gettime(CLOCK_MONOTONIC, &start);
    waitpid(child_id, &status, WUNTRACED);
    clock_gettime(CLOCK_MONOTONIC, &end); 

    double duration = end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec)/1000000000.0;
    if (!status) {
        display_results(argv, duration);
    }
    return 0;
}
