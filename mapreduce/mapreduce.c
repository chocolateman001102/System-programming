/**
 * mapreduce
 * CS 341 - Fall 2022
 */
#include "utils.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 6) {
      print_usage();
      return 1;
    }
    int num_map = atoi(argv[argc - 1]);
    if (num_map == 0) {
        print_usage();
        exit(1);
    }
    char *input = argv[1];
    char *output = argv[2];
    char *mapper = argv[3];
    char *reducer = argv[4];
    // Create an input pipe for each mapper.
    int* pipes[num_map];
    for (int i = 0; i < num_map; i++) {
      pipes[i] = calloc(2, sizeof(int));
      pipe(pipes[i]);
    }
    // Create one input pipe for the reducer.
    int r_pipes[2];
    pipe(r_pipes);
    // Open the output file.
    int file = open(output, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);


    // Start a splitter process for each mapper.
    pid_t child_n[num_map];
    for (int i = 0; i < num_map; i++) {
      pid_t child = fork();

      child_n[i] = child;
      if (child == 0) {
        char idx_str[10];
        sprintf(idx_str, "%d", i);
        close(pipes[i][0]);
        dup2(pipes[i][1], 1);
        execl("./splitter", "./splitter", input, argv[5],
                  idx_str, NULL);
        exit(1);
      }
    }

    // Start all the mapper processes.
    pid_t child_m[num_map];
    for (int i = 0; i < num_map; i++) {
        close(pipes[i][1]);
        child_m[i] = fork();
        if (!child_m[i]) {
            close(r_pipes[0]);
            dup2(pipes[i][0], 0);
            dup2(r_pipes[1], 1);
            execlp(mapper, mapper, (char*)NULL);
            exit(-1);
        }
    }
    // Start the reducer process.
    close(r_pipes[1]);
    pid_t child = fork();
    if (child == 0) {
      dup2(r_pipes[0], 0);
      dup2(file, 1);
      execl(reducer, reducer, NULL);
      exit(1);
    }
    close(r_pipes[0]);
    close(file);
    // Wait for the reducer to finish.
    for (int i = 0; i < num_map; i++) {
      int status;
      waitpid(child_n[i], &status, 0);
    }
    for (int i = 0; i < num_map; i++) {
      close(pipes[i][0]);
      int status;
      waitpid(child_m[i], &status, 0);
    }
    int status;
    waitpid(child, &status, 0);
    // Print nonzero subprocess exit codes.
    if (status)
    print_nonzero_exit_status(reducer, status);
    // Count the number of lines in the output file.
    print_num_lines(output);
    for (int i = 0; i < num_map; i++) {
      free(pipes[i]);
    }
    return 0;
}
