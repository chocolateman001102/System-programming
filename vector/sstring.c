/**
 * vector
 * CS 341 - Fall 2022
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    char* str;
};

sstring *cstr_to_sstring(const char *input) {
    sstring *ptr = (sstring*)malloc(sizeof(sstring));
    ptr->str = (char*)malloc(sizeof(char)*(strlen(input))+1);
    strcpy(ptr->str, input);
    return ptr;
}

char *sstring_to_cstr(sstring *input) {
    char *cstr = malloc(strlen(input->str) + 1);
    strcpy(cstr, input->str);
    return cstr;
}

int sstring_append(sstring *this, sstring *addition) {
    this->str = realloc(this->str,sizeof(char)*(strlen(this->str) + strlen(addition->str) + 1));
    char* str1 = this->str;
    char* str2 = addition->str;

    free(addition);
    addition = NULL;
    strcat(str1,str2);
    free(str2);
    return strlen(str1);
}

vector *sstring_split(sstring *this, char delimiter) {
    vector *vec = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    char* ptr = this->str;
    char* start = this->str;
    while(*ptr) {
        if(*(ptr) == delimiter || *(ptr + 1) == 0 ) {
            if(*(ptr) == delimiter) {
            *(ptr) = '\0';
            vector_push_back(vec,start);
                if(*(ptr+1)!= 0) {
                    start = ptr + 1;
                }
            }
            else if (*(ptr + 1) == 0) {
                vector_push_back(vec,start);
            }

        }
        ++ptr;
    }
    return vec;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    if (offset > strlen(this->str)){
        return -1;
    }
    char *pos = strstr(this->str + offset, target);
    if (pos != NULL) {
        char *new_str = malloc(strlen(this->str) + strlen(substitution) - strlen(target) + 1);
        strncpy(new_str, this->str, pos - this->str);
        strcpy(new_str + (pos - this->str), substitution);
        strcpy(new_str + (pos - this->str) + strlen(substitution), pos + strlen(target));
        free(this->str);
        this->str = new_str;
        return 0;
    } else {
        return -1;
    }
}

char *sstring_slice(sstring *this, int start, int end) {
    char* result = (char*)malloc(end-start+1);
    for (int i = start; i < end; i++){
        *(result-start + i) = this->str[i];
    }
    result[end - start] = '\0';
    return result;
}

void sstring_destroy(sstring *this) {
    free(this->str);
    this->str = NULL;
    free(this);
    this = NULL;
}
