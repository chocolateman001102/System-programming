/**
 * vector
 * CS 341 - Fall 2022
 */
#include "sstring.h"

int main(int argc, char *argv[]) {
    sstring *str = cstr_to_sstring("xxxx ewqfg r qzx ");
    sstring *str1 = cstr_to_sstring("1233232334  2321");
   
    sstring_append(str,str1);
    char* test = sstring_to_cstr(str);
    //printf("%s\n",test);
    sstring_substitute(str,120,"x","=");
    vector *vec = sstring_split(str,' ');
    //for(size_t i = 0; i < vector_size(vec);++i) {
    //    printf(" $%s$ ",vector_get(vec,i));
    //}
    sstring_destroy(str);
    vector_destroy(vec);
    free(test);
    return 0;
}
