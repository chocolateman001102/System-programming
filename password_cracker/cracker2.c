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
#include  <time.h>
#include <crypt.h>
#include  <unistd.h>

static int lines_num = 0;
static int find = 0;
static int move_next = 1;
static char* curr_line = NULL;
static int total_hash  =0;
static int global_result = 1;
static int  boardcast_end = 1;
static int boardcast_start = 1;
static int boardcast_free_curr_line = 1;
static char* global_password = NULL;
pthread_barrier_t barrier;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
typedef struct threads_t
{
    pthread_t id;
    int index;
    queue* lines_queue; 
    size_t thread_num;
    char* line;
}thread;

void* crack(void* input_thread) {

    thread* thread_in = (thread*) input_thread;
    while(1) {
    double start_time = getTime();
    double start_cpu_time = getCPUTime();
        if(lines_num == 0) {
            break;
        }

        pthread_barrier_wait(&barrier);
        pthread_mutex_lock(&lock);
        if(move_next) {
            boardcast_free_curr_line = 1;
            curr_line = queue_pull(thread_in->lines_queue);
            move_next = 0;
            lines_num--;
            //printf("Thread %d: move: %d\n", thread_in->index+1,move_next);
        }
        pthread_mutex_unlock(&lock);

        pthread_barrier_wait(&barrier);
        //printf("Thread %d: curr_line: %s\n", thread_in->index+1 , curr_line);
        char* copy = strdup(curr_line);
        int hashCount = 0;
        char* ptr = NULL;
        char* username = strtok_r(copy, " ", &ptr);
        char* hash_ = strtok_r(NULL, " ", &ptr);
        char* password_incomplete = strtok_r(NULL, " ", &ptr);
        pthread_mutex_lock(&lock);
        if(boardcast_start) {
            v2_print_start_user(username);
            boardcast_start = 0;
        }
        pthread_mutex_unlock(&lock);
        
        int password_incomplete_length = strlen(password_incomplete);
        int unknown_letter_count = password_incomplete_length - getPrefixLength(password_incomplete) - 1;
        //printf("password_incomplete: %s, unknown_letter_count :%d\n",password_incomplete,unknown_letter_count);
        long *start_index= malloc(sizeof(long));
        long *count = malloc(sizeof(long));
        getSubrange(unknown_letter_count, thread_in->thread_num, thread_in->index + 1, start_index, count);
        char* temp_password =  (char*)malloc( ((int)strlen(password_incomplete))* sizeof(char) + 1);

        strncpy(temp_password, password_incomplete, getPrefixLength(password_incomplete));
        temp_password[(int)strlen(password_incomplete)-1] = '\0';
        char* unknown_part = (char*) malloc(unknown_letter_count*sizeof(char)+1);
        memset(unknown_part,'-',unknown_letter_count);
        unknown_part[unknown_letter_count] = '\0';
        setStringPosition(unknown_part,*start_index);
        //printf("unknown part num: %d, string :%s\n",unknown_letter_count,unknown_part);

        strncpy(temp_password +  getPrefixLength(password_incomplete), unknown_part, unknown_letter_count);


        v2_print_thread_start(thread_in->index + 1, username, *start_index, temp_password);

        pthread_barrier_wait(&barrier);
        char* cracked_data = NULL;
        struct crypt_data c_target;
        c_target.initialized = 0;
        int result = 1;
        for(int i = 0; i < *count; ++i) {

            pthread_mutex_lock(&lock);
            if( global_result == 0 ) {  
                pthread_mutex_unlock(&lock);                     
                break;
            }
            pthread_mutex_unlock(&lock); 
            strncpy(temp_password +  getPrefixLength(password_incomplete), unknown_part, unknown_letter_count);
        
            cracked_data = crypt_r(temp_password, "xx", &c_target);

            ++hashCount;
            if(strcmp(cracked_data, hash_) == 0)
            {

                pthread_mutex_lock(&lock);
                ++find;
                result = 0;
                global_result = 0;
                global_password = temp_password;
                pthread_mutex_unlock(&lock);
                break;
            }
            result = incrementString(unknown_part);
            if(result == 0 || i == *count -1 ) {
                result = 2;
                break;
            }
        }


        v2_print_thread_result( thread_in->index +1, hashCount, result);
        pthread_barrier_wait(&barrier);
        double timeUsed = getTime() - start_time;
        double totalCPUTime = getCPUTime() - start_cpu_time;
        pthread_mutex_lock(&lock);
        total_hash += hashCount;
        pthread_mutex_unlock(&lock);
        pthread_barrier_wait(&barrier);

        pthread_mutex_lock(&lock);

        if(boardcast_end) {
            boardcast_end = 0;
            v2_print_summary(username, global_password, total_hash,timeUsed, totalCPUTime, global_result);
            //printf("start_time reset\n");
        }
        pthread_mutex_unlock(&lock);
        pthread_barrier_wait(&barrier);

        pthread_mutex_lock(&lock);
        global_result = 1;
        move_next = 1;
        boardcast_end = 1;
        boardcast_start = 1;
        pthread_mutex_unlock(&lock);

        free(unknown_part);
        free(temp_password);
        free(start_index);
        free(count); 
        free(copy);
        pthread_barrier_wait(&barrier);
        pthread_mutex_lock(&lock);

        if(boardcast_free_curr_line) {
            boardcast_free_curr_line = 0;
            free(curr_line);
        }
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}
int start(size_t thread_count) {
    queue* lines = queue_create(-1);
    char* buffer = NULL;
    size_t buffer_size = 0;
    ssize_t valid = 0;
    pthread_barrier_init(&barrier, NULL, thread_count);

    valid = getline(&buffer, &buffer_size, stdin);
    while(valid != -1 && !feof(stdin))
    {
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


    //array of threads which process at the same time
    thread* thread_array = (thread*)malloc(thread_count * sizeof(thread));

    //cracking
    for(size_t i = 0; i < thread_count; i++)
    {
        thread_array[i].thread_num= thread_count;
        thread_array[i].index = i;
        thread_array[i].lines_queue = lines;
        pthread_create(&(thread_array[i].id), NULL, crack, &thread_array[i]);
        
    }
    

    for(size_t i = 0; i < thread_count; i++)
    {
        pthread_join(thread_array[i].id, NULL);
    }

    free(thread_array);
    free(buffer);
    queue_destroy(lines);
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&lock);
    return 0; 
}
