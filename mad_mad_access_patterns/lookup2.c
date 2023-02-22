/**
 * mad_mad_access_patterns
 * CS 341 - Fall 2022
 */
#include "tree.h"
#include "utils.h"
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/
BinaryTreeNode *wordBinarySearch(uint32_t offset, char *addr, char *search_word) {

  if (offset == 0) 
  {
    printNotFound(search_word);
    return NULL;
  }
    BinaryTreeNode *node = (BinaryTreeNode *) (addr+offset);
    BinaryTreeNode *word_node;

  if (strcmp(search_word, node->word) == 0) {
    return node;
  } else if (strcmp(search_word, node->word) < 0) {

    if ((word_node = wordBinarySearch(node->left_child, addr, search_word))) {
        return word_node;
      }

  } else {

    if ((word_node = wordBinarySearch(node->right_child, addr, search_word))) {
      return word_node;
    }

  }

  return NULL;
}


int main(int argc, char **argv) {
  if (argc <= 2) {
    printArgumentUsage();
    exit(1);
  }

  FILE* f = fopen(argv[1], "r");
  if (f == NULL) {
      openFail(argv[1]);
      exit(2);
  }

  fseek(f, 0, SEEK_END);
  size_t file_size = ftell(f);
  rewind(f);

  char* address = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fileno(f), 0);
  if (address == (void *)-1) {
    mmapFail(argv[1]);
    exit(2);
  }

  if (strncmp(address, BINTREE_HEADER_STRING, 4)) {
    formatFail(argv[1]);
    exit(2);
  }

  for (int i = 2; i < argc; i++) {
    BinaryTreeNode *bnry_tree = wordBinarySearch(4, address, argv[i]);
    if (bnry_tree) {
      printFound(bnry_tree->word, bnry_tree->count, bnry_tree->price);
    } else {
      printNotFound(argv[i]);
    }
  }

  fclose(f);
  return 0;
}
