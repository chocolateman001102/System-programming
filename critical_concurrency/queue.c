/**
 * critical_concurrency
 * CS 341 - Fall 2022
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    queue *this = malloc(sizeof(queue));
    if (this == NULL)
        return NULL;
    this->head = NULL;
    this->tail = NULL;
    this->size = 0;
    this->max_size = max_size;
    pthread_cond_init(&(this->cv), NULL);
    pthread_mutex_init(&(this->m), NULL);
    return this;
}

void queue_destroy(queue *this) {
    if (this == NULL){
    return;
    }
    queue_node *tep = this->head;
    while (tep != NULL) {
        queue_node *temp = tep;
        tep = tep->next;
        free(temp);
    }
    pthread_cond_destroy(&(this->cv));
    pthread_mutex_destroy(&(this->m));
    free(this);
}

void queue_push(queue *this, void *data) {
    pthread_mutex_lock(&(this->m));
    while (this->max_size > 0 && this->size >= this->max_size) {
        pthread_cond_wait(&this->cv, &this->m);
    }
    queue_node *new_node = malloc(sizeof(queue_node));
    new_node->data = data;
    new_node->next = NULL;

    if (this->size == 0) {
        this->head = new_node;
        this->tail = new_node;
    } else {
        this->tail->next = new_node;
        this->tail = new_node;
    }
    ++(this->size);
    if (this->size > 0) {
      pthread_cond_broadcast(&this->cv);
    }
    pthread_mutex_unlock(&(this->m));
}

void *queue_pull(queue *this) {
    pthread_mutex_lock(&this->m);
    while (this->size == 0) {
      pthread_cond_wait(&this->cv, &this->m);
    }
    void *data = this->tail->data;
    queue_node *curr = this->tail;
    this->tail = this->tail->next;
    
    if (!this->tail) {
      this->head = NULL;
    }
    this->size--;

    if (this->max_size > 0 && this->size < this->max_size) {
      pthread_cond_broadcast(&this->cv);
    }
    pthread_mutex_unlock(&this->m);
    free(curr);
    return data;
}
