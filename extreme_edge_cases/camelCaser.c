/**
 * extreme_edge_cases
 * CS 241 - Fall 2022
 */
#include <stdio.h>
#include "camelCaser.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

char **camel_caser(const char *input_str) {
    if(input_str == NULL) {
        return NULL;
    }

    char **result = NULL;
    char *sentence = NULL;
    sentence= (char *) malloc(sizeof(char));
    result= (char **) malloc(sizeof(char *));
    int array_len  = 0;
    int sentence_len = 0;
    while(*input_str) {
        int ascii_curr = *input_str;
        if(!ispunct(ascii_curr)) {
               
            sentence[sentence_len] = *input_str ;
            ++sentence_len;
            sentence= (char *) realloc(sentence,(sentence_len+1)*sizeof(char)+1);
        }
        else if(ispunct(ascii_curr)){
            result= (char **) realloc(result, (array_len+2) * sizeof(char *));
            *(sentence + sentence_len) = '\0';
            char* new_one = (char*)malloc((sentence_len +2)* sizeof(char));
            const char* str = sentence;
            strcpy(new_one, str);
            result [array_len] = new_one;
            //*(result + (array_len +1)* sizeof(char)) = NULL;
            result[array_len +1] = NULL;
            
            ++array_len;
            sentence_len = 0;
            
        }
        ++input_str;        
    }
    free(sentence);
    sentence = NULL;

    int j =0;
    while(*result) {
        int is_first = 1;
        int k = 0;
        while(**result) {
            
            if(is_first) {
                if(isupper(*((*result)))) {
                    *((*result)) += 32;
                    is_first = 0;
                } else if(islower(*((*result)))) {
                    is_first = 0;
                }
            }
            if(isspace(**result)) {
                if(isdigit(*((*result) - sizeof(char)))) {
                    is_first = 0;
                }
                if(islower(*((*result) + sizeof(char)))&& !is_first ){
                    *((*result) + sizeof(char)) -= 32;
                }
            }
            (*result)++;
            k++;
        }
        while(k) {
            (*result)--;
            k--;
        }
        result++;
        j++;
    }
    while(j) {
        result--;
        j--;
    }

    int i = 0;
    while(*result) {
    char *new = *result;
    int reverse = 0;
        while(**result != 0) {
            //printf("%c \n",**result);
            if(!isspace(**result)) {
                *new++ = **result;
                reverse++;
            }  
        ++(*result);
        }
    //printf("%s \n",new);
        *new = 0;
        while(reverse) {
            new--;
            reverse--;
        }
        *result = new;
        result++;
        i++;
    }
    while(i) {
        result--;
        i--;
    }
    
   
    return result;
}


void destroy(char **result) {
    int reverse = 0;
    while(*result) {
        free(*result);
        *result = NULL;
        ++result;
        ++reverse;
    }
    while(reverse) {
        --result;
        --reverse;
    }
    free(result);
    result = NULL;
    return;
}