/**
 * password_cracker
 * CS 341 - Fall 2022
 */
#include <stdio.h>
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "includes/queue.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <crypt.h>
static int lines_num = 0;
static int find = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
typedef struct threads_t
{
    pthread_t id;
    int index;
    queue* lines_queue;    
}thread;

void* crack(void* input_thread) {
    double start_time = getTime();

    thread* thread_in = (thread*) input_thread;

    while(1) {
        pthread_mutex_lock(&lock);
        lines_num--;
        if(lines_num < 0)
        {
            pthread_mutex_unlock(&lock);
            break;
        }
        pthread_mutex_unlock(&lock);
        int hashCount = 0;
        pthread_mutex_lock(&lock);
        char* line = queue_pull(thread_in->lines_queue);
        pthread_mutex_unlock(&lock);
        char* ptr = NULL;
        char* username = strtok_r(line, " ", &ptr);
        char* hash_ = strtok_r(NULL, " ", &ptr);
        char* password_incomplete = strtok_r(NULL, " ", &ptr);
        int known_part = getPrefixLength(password_incomplete);
        v1_print_thread_start(thread_in -> index + 1 , username);

        char* temp_password =  (char*)malloc( ((int) strlen(password_incomplete) + 1)* sizeof(char));
        temp_password[(int) strlen(password_incomplete)-1] = '\0';
        strncpy(temp_password, password_incomplete, getPrefixLength(password_incomplete));
        int unknown_letter_count = strlen(password_incomplete) - getPrefixLength(password_incomplete) -1 ;
        char* unknown_part = (char*) malloc(unknown_letter_count*sizeof(char) +1);
        memset(unknown_part,'a',unknown_letter_count);
        unknown_part[unknown_letter_count] = '\0';

        char* cracked_data = NULL;
        struct crypt_data c_target;
        c_target.initialized = 0;
        int is_fail = 1;

        while(is_fail) {
            strncpy(temp_password + known_part, unknown_part, strlen(password_incomplete)-known_part-1);
            cracked_data = crypt_r(temp_password, "xx", &c_target);
            //printf("%d, %s \n",hashCount, unknown_part);
            ++hashCount;
            if(strcmp(cracked_data, hash_) == 0)
            {
                pthread_mutex_lock(&lock);
                ++find;
                pthread_mutex_unlock(&lock);
                break;
            }
            is_fail = incrementString(unknown_part);
        }
        double timeUsed = getTime() - start_time;
        v1_print_thread_result(thread_in->index +1 , username, temp_password, hashCount, timeUsed, !is_fail);
        free(temp_password);
        free(line);
        free(unknown_part);
    
    }
    return NULL;
}
int start(size_t thread_count) {
    queue* lines = queue_create(-1);
    (void) lines;
    char* buffer = NULL;
    size_t buffer_size = 0;
    ssize_t valid = 0;
    
    valid = getline(&buffer, &buffer_size, stdin);

    while(valid != -1 && !feof(stdin)) {
        queue_push(lines, strdup(buffer));
        lines_num++;
        valid = getline(&buffer, &buffer_size, stdin);
    }

    if(valid != -1) {
        char* extend = "\n";
        strcat(buffer,extend);
        queue_push(lines, strdup(buffer));
        lines_num++;
    }
    int total_lines = lines_num;

    //array of threads which process at the same time
    thread* thread_array = (thread*)malloc(thread_count * sizeof(thread));

    //cracking
    for(size_t i = 0; i < thread_count; i++)
    {
        thread_array[i].index = i;
        thread_array[i].lines_queue = lines;
        pthread_create(&(thread_array[i].id), NULL, crack, &thread_array[i]);
    }

    for(size_t i = 0; i < thread_count; i++)
    {
        pthread_join(thread_array[i].id, NULL);
    }
    v1_print_summary(find, total_lines-find) ;

    free(thread_array);
    free(buffer);
    queue_destroy(lines);

    pthread_mutex_destroy(&lock);
    return 0; 
}
