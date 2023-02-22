/**
 * nonstop_networking
 * CS 341 - Fall 2022
 */
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>


#include "common.h"

int connect_to_server(const char *host, const char *port);
void request_server(char **args, int socket, verb method);
void get_response(char **args, int socket, verb method);
char **parse_args(int argc, char **argv);
verb check_args(char **args);


int main(int argc, char **argv) {
    char** arugments = parse_args(argc, argv);
    if (!arugments) {
        print_client_usage();
        free(arugments);
        exit(1);
    }
    char* host = arugments[0];
    char* port = arugments[1];
    verb method = check_args(arugments);

    int sock_fd = connect_to_server(host,port);

    request_server(arugments, sock_fd, method);
    if (shutdown(sock_fd, SHUT_WR) == -1) {
        perror("shutdown");
    }

    get_response(arugments,sock_fd, method);

    if (shutdown(sock_fd, SHUT_RD) == -1) {
        perror("shutdown");
    }
    close(sock_fd);
    free(arugments);
    return 0;

}



void request_server(char **args, int sock_fd, verb method) {
    char* remote_file = args[3]; 
    char* local_file = args[4]; 
    char *request_to_send;
    if (method == LIST) {
        write(sock_fd, "LIST\n", 5);
    } else {//quest string of LIST GET PUT, "METHOD REMOTE_FILE\n"
        request_to_send = calloc(1, strlen(args[2]) + strlen(remote_file) + 3);
        sprintf(request_to_send, "%s %s\n", args[2], remote_file);
        ssize_t len = strlen(request_to_send);
        if (write_all_to_socket(sock_fd, request_to_send, len) < len) {
            print_connection_closed();
            exit(1);
        }
        free(request_to_send);
    }

    if (method == PUT) {//Check if local file exist
    struct stat st;
    if(stat(local_file, &st) == -1) {
        exit(1);
    }

    size_t file_size  = st.st_size;
    write_all_to_socket(sock_fd, (char*)&file_size , sizeof(size_t));//write size to remote file
    FILE *local_f = fopen(local_file, "r");
    if (!local_f) {
        fprintf(stdout, "cant open local file\n");
        exit(1);
    }
    size_t written_bytes = 0;
    while (written_bytes < file_size ) {
        ssize_t size_each_w;
        if((file_size  - written_bytes) > 1024) {
            size_each_w = 1024;
        } else {
            size_each_w = file_size  - written_bytes;
        }
        char buffer[size_each_w + 1];
        fread(buffer, 1, size_each_w, local_f);
        if (write_all_to_socket(sock_fd, buffer, size_each_w) < size_each_w) {
            print_connection_closed();
            exit(1);
        }
        written_bytes += size_each_w;
    }
    fclose(local_f);
  } 
}

void get_response(char **args, int sock_fd, verb method) {

    //"ok\n + 0 ""= 4
    char *buffer = calloc(1,4);
    
    //"ok\n" = 3
    size_t bytes_rd = read_all_from_socket(sock_fd, buffer, 3);
    //printf("now buffer is %s\n",buffer);
    
    if (strcmp(buffer, "OK\n") == 0) {
        fprintf(stdout, "%s", buffer);
        if (method == PUT) {
            print_success();
        } else if (method == DELETE) {
            print_success();
        }
        else if (method == LIST) {
        size_t size;
        read_all_from_socket(sock_fd, (char *)&size, sizeof(size_t));
        char buffer_f[size + 20 + 1];
        memset(buffer_f, 0, size + 20 + 1);
        bytes_rd = read_all_from_socket(sock_fd, buffer_f, size + 20);

        //print error
        if (bytes_rd == 0 && bytes_rd != size) {
            print_connection_closed();
            exit(1);
        } else if (bytes_rd < size) {
            print_too_little_data();
            exit(1);
        } else if (bytes_rd > size) {
            print_received_too_much_data();
            exit(1);
        }

        fprintf(stdout, "%zu%s", size, buffer_f);
        
        } else if (method == GET) {
            FILE *lf = fopen(args[4], "w+");
            if (!lf) {
                perror(NULL);
                exit(1);             
            }

            size_t size;

            read_all_from_socket(sock_fd, (char*)&size, sizeof(size_t));

            size_t bytes_read = 0;
            while (bytes_read < size + 9) {
                size_t each_read ;
                if((size + 9 - bytes_read) > 1024) {
                    each_read = 1024;
                } else {
                    each_read = size + 9 - bytes_read;
                }
                char buffer_f[1025] = {0};

                size_t write_count = read_all_from_socket(sock_fd, buffer_f, each_read);

                fwrite(buffer_f, 1, write_count, lf);
                bytes_read += write_count;
                if (write_count == 0){
                    break;
                }
            }

            //print error
            if (bytes_read == 0 && bytes_read != size) {
                print_connection_closed();
                exit(1);
            } else if (bytes_read < size) {
                print_too_little_data();
                exit(1);
            } else if (bytes_read > size) {
                print_received_too_much_data();
                exit(1);
            }
            fclose(lf);
        } 
    } else {
    
        buffer = realloc(buffer,1025);
        //printf("now buffer is %s\n",buffer);
        ssize_t response_size = read_all_from_socket(sock_fd, buffer + 3, 3);
        //printf("now buffer is %s\n",buffer);
        if (response_size == -1) {
            print_connection_closed();
            exit(1);
        }
        if (strcmp(buffer, "ERROR\n") == 0) {
            ssize_t error_size = read_all_from_socket(sock_fd, buffer + 6, 1024);
            if (error_size == -1)  {
                print_connection_closed();
                exit(1);
            }
            print_error_message(buffer);
        } else {
            print_invalid_response();
        }
    }
    free(buffer);
}


int connect_to_server(const char *host, const char *port) {
    static struct addrinfo hints, *result;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int rst = getaddrinfo(host, port, &hints, &result);
    if (rst != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rst));
        freeaddrinfo(result);
        result = NULL;
        exit(1);
    }
    int sock_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock_fd == -1) {
        freeaddrinfo(result);
        result = NULL;
        perror("socket");
        exit(1);
    }

  int ok = connect(sock_fd, result->ai_addr, result->ai_addrlen);
  if (ok == -1) {
    perror("connect");
    freeaddrinfo(result);
    result = NULL;
    exit(1);
  }
    freeaddrinfo(result);
    result = NULL;
    return sock_fd;
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
