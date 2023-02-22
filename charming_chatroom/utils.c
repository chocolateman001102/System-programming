/**
 * charming_chatroom
 * CS 341 - Fall 2022
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    size_t nsize = htonl(size);
    return write_all_to_socket(socket, (char*)&nsize, MESSAGE_SIZE_DIGITS);
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    size_t total_read = 0;
    while (total_read < count) {
      ssize_t result = read(socket, buffer + total_read, count - total_read);
      if (result == 0) {
        return 0;
      }
      if (result == -1 && errno == EINTR) {
        continue;
      }
      if (result == -1) {
        return -1;
      }
      total_read += result;
    }
    return total_read;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    size_t total_write = 0;
    while (total_write < count) {
      ssize_t result = write(socket, buffer + total_write, count - total_write);
      if (result == 0) {
        return 0;
      }
      if (result == -1 && errno == EINTR) {
        continue;
      }
      if (result == -1) {
        return -1;
      }
      total_write += result;
    }

    return total_write;
}
