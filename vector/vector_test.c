/**
 * vector
 * CS 341 - Fall 2022
 */
#include "vector.h"
#include <stdio.h>
void print_vector(vector *v);
int main(int argc, char *argv[]) {
    vector *v= vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    vector_push_back(v, "0");    
    vector_push_back(v, "1");
    vector_push_back(v, "2");
    vector_push_back(v, "3");
    vector_push_back(v, "4");
    vector_push_back(v, "0");    
    vector_push_back(v, "1");
    vector_push_back(v, "2");
    vector_push_back(v, "3");
    vector_push_back(v, "4");    vector_push_back(v, "0");    
    vector_push_back(v, "1");
    vector_push_back(v, "2");
    vector_push_back(v, "3");
    vector_push_back(v, "4");    vector_push_back(v, "0");    
    vector_push_back(v, "1");
    vector_push_back(v, "2");
    vector_push_back(v, "3");
    vector_push_back(v, "4");    vector_push_back(v, "0");    
    vector_push_back(v, "1");
    vector_push_back(v, "2");
    vector_push_back(v, "3");
    vector_push_back(v, "4");
    vector_erase(v,1);
    print_vector(v);
    vector_destroy(v);

    return 0;
}

void print_vector(vector *v){
    char **begin = (char **)vector_begin(v);
    char **end = (char **)vector_end(v);
    while (begin < end) {
        printf("%s\n",*begin);
        begin++;
    }
    printf("size: %zu\n", vector_size(v));
    printf("capacity: %zu\n", vector_capacity(v));

}
