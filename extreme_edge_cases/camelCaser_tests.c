/**
 * extreme_edge_cases
 * CS 341 - Fall 2022
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Implement me!

    char *input0 = NULL;
    if (camelCaser(input0) != NULL){
        return 0;
    }


    char *input1 = "  ";
    char **output1 = camelCaser(input1);
    if (output1[0] != NULL){
        destroy(output1);
        return 0;
    }
    destroy(output1);
    

    char *input2 = "";
    char **output2 = camelCaser(input2);
    if (output2[0] != NULL){
        destroy(output2);
        //printf("here! 2\n");
            return 0;
    }
    destroy(output2);

    char *input4 = "";
    char** output4 = camelCaser(input4);
    if (output4[0] != NULL){
        //printf("here! 4\n");
        return 0;
    }
    destroy(output4);

    char *input3 = "hello. welcome to cs241...   ..";
    char** output3 = camelCaser(input3);
    char *correct_3[] = {"hello","welcomeToCs241","","","","",NULL};
    for(unsigned long i= 0; i < sizeof(correct_3)/sizeof(char*) - 1;++i) {
        if(strcmp(output3[i], correct_3[i])) {
            //printf("%s  %s\n",output3[i],correct_3[i]);
            destroy(output3);
            return 0;
        }
    }
    if(output3[sizeof(correct_3)/sizeof(char*) - 1] != NULL) {
        destroy(output3);
        //printf("here! 3\n");
        return 0;
    }
    destroy(output3);


  char *input5 = "  This tt xxxx;   fefesc   feqq.    ";
  char **output5 = camelCaser(input5);
  char *correct_5[] = {"thisTtXxxx","fefescFeqq",NULL};
  //printf("%lu",sizeof(correct_5)/sizeof(char*));
    for(unsigned long i= 0; i < sizeof(correct_5)/sizeof(char*) - 1;++i) {
        if(strcmp(output5[i], correct_5[i])) {
            destroy(output5);
            //printf("here! 5\n");
            return 0;
        }
    }
  destroy(output5);


  char *input9 = "123 This.";
  char **output9 = camelCaser(input9);
  char *correct_9[] = {"123This",NULL};
    for(unsigned long i= 0; i < sizeof(correct_9)/sizeof(char*) - 1;++i) {
        if(strcmp(output9[i], correct_9[i])) {
            destroy(output9);
            //printf("here! 9\n");
            return 0;
        }
    }
  destroy(output9);

    char *input10 = "123This.";
  char **output10 = camelCaser(input10);
  char *correct_10[] = {"123this",NULL};
    for(unsigned long i= 0; i < sizeof(correct_10)/sizeof(char*) - 1;++i) {
        if(strcmp(output10[i], correct_10[i])) {
            destroy(output10);
            //printf("here! 10\n");
            return 0;
        }
    }
        if(output10[sizeof(correct_10)/sizeof(char*) - 1] != NULL) {
        destroy(output10);
        //printf("here! 10\n");
        return 0;
    }
  destroy(output10);

  char *input12 = ".";
  char **output12 = camelCaser(input12);
  char *correct_12[] = {"",NULL};
    for(unsigned long i= 0; i < sizeof(correct_12)/sizeof(char*) - 1;++i) {
        if(strcmp(output12[i], correct_12[i])) {
            destroy(output12);
            //printf("here! 12\n");
            return 0;
        }
    }
        if(output12[sizeof(correct_12)/sizeof(char*) - 1] != NULL) {
        destroy(output12);
        //printf("here! 12\n");
        return 0;
    }
  destroy(output12);

  char * input13 =
      "erwavcxzvcfzggr  rtgreg dxfv x vcsv ewt ttrtrtrt rt4r hbtyhhyngvc  fgdg grgq fef cxv ew rf "
      "fewffdsvcvxv  .vfvrgrtbg bdf vcvs dfq f ewfrev bvbt tyt tw  vc x  "
      "dwadawddscac .cdc dgreg  sd fdsf dsvsxfqwererr rtbbgf dgfg fdg fggd gfd df gfd .";
char *correct_13[] = {"erwavcxzvcfzggrRtgregDxfvXVcsvEwtTtrtrtrtRt4rHbtyhhyngvcFgdgGrgqFefCxvEwRfFewffdsvcvxv",
        "vfvrgrtbgBdfVcvsDfqFEwfrevBvbtTytTwVcXDwadawddscac",
        "cdcDgregSdFdsfDsvsxfqwererrRtbbgfDgfgFdgFggdGfdDfGfd",
        NULL};
  char **output13 = camelCaser(input13);
    for(unsigned long i= 0; i < sizeof(correct_13)/sizeof(char*) - 1;++i) {
        if(strcmp(output13[i], correct_13[i])) {
            destroy(output13);
            //printf("here! 13\n");
            return 0;
        }
    }
    if(output13[sizeof(correct_13)/sizeof(char*) - 1] != NULL) {
        destroy(output13);
        //printf("here! 13\n");
        return 0;
    }
  destroy(output13);


    return 1;
}
