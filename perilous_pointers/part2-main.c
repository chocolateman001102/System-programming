/**
 * perilous_pointers
 * CS 341 - Fall 2022
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);
    int i = 132;
    int *value = &i;
    second_step(value);
    int **arr = (int**)malloc(sizeof(int*));
    *arr = (int*)malloc(sizeof(int));
    arr[0][0] = 8942;
    double_step(arr);
    free(*arr);
    free(arr);
    *arr = NULL;
    arr = NULL;

    char *four = malloc(5 + sizeof(int));
    * (int *)(four + 5) = 15;
    strange_step(four);
    free(four);
    four = NULL;


    char value5[4];
    value5[3] = '\0';
    empty_step(value5);
    void* s;
    char s2[4];
    s2[3] = 'u';
    s = s2;
    two_step(s,s2);
    char *first = "c";
    char *second = first +2;
    char *third = second +2;
    three_step(first,second,third);


    //third[3] == second[2] + 8 && second[2] == first[1] + 8
    char* first1 = "abre";
    char* second1 = "abj";
    char* third1 = "abjr";
    step_step_step(first1,second1,third1);
    //*a == b && b > 0
    char *a = "1";
    int b = 49 ;
    it_may_be_odd(a, b);
    char che[100] = "CS2fefe41,CS241";
    tok_step(che);
    int q = 0x2001;
    int* oran = &q;
    //orange != NULL && orange == blue && ((char *)blue)[0] == 1 &&
               //*((int *)orange) % 3 == 0
    the_end(oran,oran);
    return 0;
}
