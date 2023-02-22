/**
 * nonstop_networking
 * CS 341 - Fall 2022
 */
#include "common.h"


ssize_t write_all_to_socket(int socket, const char* buffer, size_t target_count) {
    size_t ct = 0;
    while (ct != target_count) {
        ssize_t ret_bytes = write(socket, buffer + ct, target_count - ct);
        if (ret_bytes > 0) {
            ct += ret_bytes;
        } else if (ret_bytes == -1 && errno == EINTR) {
            continue;
        } else {            
            return -1;
        }
    }
    return ct;
}

ssize_t read_all_from_socket(int socket, char* buffer, size_t target_count) {
    size_t ct = 0;
    while (ct != target_count) {
        ssize_t bytes_read = read(socket, buffer + ct, target_count - ct);
        if (bytes_read == 0) {
            return ct;
        } else if (bytes_read > 0) {
            ct += bytes_read;
        } else if (bytes_read == -1 && errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }
    return ct;
}
