/**
 * mad_mad_access_patterns
 * CS 341 - Fall 2022
 */
#include "tree.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/
int searchTree(FILE* f, uint32_t offset, char* search_word);

int searchTree(FILE* f, uint32_t offset, char* search_word) {
  if (offset == 0) {
    printNotFound(search_word);
    return 0;
  }

  fseek(f, offset, SEEK_SET);
  BinaryTreeNode bnry_tree;
  fread(&bnry_tree, sizeof(BinaryTreeNode), 1, f);
  fseek(f, offset+sizeof(BinaryTreeNode), SEEK_SET);

  char word[10];
  fread(word, 10, 1, f);
    if (strcmp(search_word, word) == 0) {
        printFound(word, bnry_tree.count, bnry_tree.price);
        return 1;
    } else if (strcmp(search_word, word) < 0) {
        searchTree(f, bnry_tree.left_child, search_word);
    } else {
        searchTree(f, bnry_tree.right_child, search_word);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc <= 2) {
        printArgumentUsage();
        exit(1);
    }

    FILE* f = fopen(argv[1], "r");
    char *file_name = argv[1];

    if (f == 0) {
        openFail(file_name);
        exit(2);
    }

    char checksum[5];
    memset(checksum, 0, 5);
    fread(checksum, 1, 4, f);

    if (strcmp(checksum,BINTREE_HEADER_STRING)) {
        formatFail(file_name);
        fclose(f);
        exit(2);
    }

    for (int i = 2; i < argc; i++) {
        rewind(f);
        int found = searchTree(f, 4, argv[i]);
        if (!found) {
          printNotFound(argv[i]);
        }
    }
    fclose(f);
    return 0;
}
