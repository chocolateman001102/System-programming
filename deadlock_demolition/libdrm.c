/**
 * deadlock_demolition
 * CS 341 - Fall 2022
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>


int detect_cycle(void* cur);

struct drm_t {pthread_mutex_t m;};
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static graph* g;
static set* s;


drm_t *drm_init() {
    drm_t* drm = malloc(sizeof(drm_t));
    if (!drm) {
        return NULL;
    }
    pthread_mutex_init(&(drm->m), NULL);

    pthread_mutex_lock(&m);
    if (!g) {
        g = shallow_graph_create();
    }
    graph_add_vertex(g, drm);
    pthread_mutex_unlock(&m);
    return drm;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&m);
    if (!graph_contains_vertex(g, thread_id) || !graph_contains_vertex(g, drm)) {
      pthread_mutex_unlock(&m);
      return 0;
    }
    if (graph_adjacent(g, drm, thread_id)) {
      graph_remove_edge(g, drm, thread_id);
      pthread_mutex_unlock(&drm->m);
    }
    pthread_mutex_unlock(&m);
    return 1;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&m);
    graph_add_vertex(g, thread_id);
    if (graph_adjacent(g, drm, thread_id)) {
      pthread_mutex_unlock(&m);
      return 0;
    }
    graph_add_edge(g, thread_id, drm);
    if (detect_cycle(thread_id)) {
      graph_remove_edge(g, thread_id, drm);
      pthread_mutex_unlock(&m);
      return 0;
    } else {
      pthread_mutex_unlock(&m);

      pthread_mutex_lock(&drm->m);
      pthread_mutex_lock(&m);
      graph_remove_edge(g, thread_id, drm);
      graph_add_edge(g, drm, thread_id);
      pthread_mutex_unlock(&m);
      return 1;
    }
}

void drm_destroy(drm_t *drm) {
    graph_remove_vertex(g, drm);
    pthread_mutex_destroy(&drm->m);
    free(drm);
    pthread_mutex_destroy(&m);
    return;
}

int detect_cycle(void* cur) {
    if (!s) s = shallow_set_create();
    if (set_contains(s, cur)) {
        set_destroy(s);
        s = NULL;
        return 1;
    }
    set_add(s, cur);
    vector* neighbors = graph_neighbors(g, cur);
    for (size_t i = 0; i < vector_size(neighbors); i++) {
        if (detect_cycle(vector_get(neighbors, i))) {
            return 1;
        }
    }
    vector_destroy(neighbors);
    set_destroy(s);
    s = NULL;
    return 0;
}