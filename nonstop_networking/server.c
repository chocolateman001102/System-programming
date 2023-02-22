/**
 * nonstop_networking
 * CS 341 - Fall 2022
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h> 

#include "format.h"
#include "common.h"
#include "includes/dictionary.h"
#include "includes/vector.h"

typedef struct client_info_t {
    int state;
    verb method;
    char filename[256]; 
    char header[1024];
} client_info;


void run_server(char *port);
int response_client(int clientfd);
void close_server();
void epoll_mod_out(int clientfd);

void parse_header(int clientfd, client_info* client_info);
int get_hdlr(int clientfd, client_info* client_info);
int delete_hdlr(int clientfd, client_info* client_info);
int list_hldr(int clientfd);

void sigpipe_IGN(int signal);


static int sock_fd;
static char* temp_dir;
static int epoll_fd;

static dictionary* clientfd_to_clientinfo;
static dictionary* filename_to_filesize;
static vector* file_name_list;
static size_t filename_size; 

int main(int argc, char **argv) {
    if (argc != 2) {
        print_server_usage();
        exit(1);
    }
    char* port = argv[1];
    
    signal(SIGPIPE, sigpipe_IGN);
    struct sigaction int_act;
    memset(&int_act, 0, sizeof(int_act));

    int_act.sa_handler = close_server;
    if (sigaction(SIGINT, &int_act, NULL) < 0) {
        perror("sigaction error");
        exit(1);
    }

    //temp directory
    char template[] = "XXXXXX";
    temp_dir = mkdtemp(template);
    print_temp_directory(temp_dir);
    
    clientfd_to_clientinfo = int_to_shallow_dictionary_create();
    filename_to_filesize = string_to_unsigned_int_dictionary_create();
    file_name_list = string_vector_create();

    run_server(port);
}

void run_server(char *port) {
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket err");
        exit(1);
    }
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int retval = getaddrinfo(NULL, port, &hints, &result);
    if (retval != 0) {
        freeaddrinfo(result);
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
        exit(1);
    }
    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt1 err");
        exit(1);
    }
    opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        perror("setsockopt2 err");
        exit(1);
    }

    if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
        freeaddrinfo(result);
        perror("bind err");
        exit(1);
    }
    if (listen(sock_fd, 128) != 0) {
        freeaddrinfo(result);
        perror("listen err");
        exit(1);
    }

    freeaddrinfo(result);
    epoll_fd = epoll_create(128);
	if (epoll_fd == -1) {
        perror("epoll_create err");
        exit(1);
    }

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = sock_fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev);
    struct epoll_event e_events[128];

    while (1) {
        int num_events = epoll_wait(epoll_fd, e_events, 128, -1);
        if (num_events == -1) {
            if (errno != EINTR) {
                perror("epoll_wait err");
                exit(1);
            }
        }
        for (int i = 0; i < num_events; i++) {
            if (e_events[i].data.fd == sock_fd) {
                int clientfd = accept(sock_fd, NULL, NULL);
                if (clientfd < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break;
                    }
                    perror("accept err");
                    exit(1);
                }
                struct epoll_event new_epoll_event;
                memset(&new_epoll_event, 0, sizeof(new_epoll_event));
                new_epoll_event.events = EPOLLIN;
                new_epoll_event.data.fd = clientfd;

                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientfd, &new_epoll_event) == -1) {
                    perror("epoll_ctl err");
                    exit(1);
                }
                client_info* info_0 = calloc(1, sizeof(client_info));
                dictionary_set(clientfd_to_clientinfo, &clientfd, info_0);
            } else {
                int result = response_client(e_events[i].data.fd);
                if(result) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e_events[i].data.fd, NULL);
                    dictionary_remove(clientfd_to_clientinfo, &(e_events[i].data.fd));
                    shutdown(e_events[i].data.fd, SHUT_RDWR);
                    close(e_events[i].data.fd);
                }
            }
        }
    }
}



void sigpipe_IGN(int signal) {
    if (signal == SIGPIPE) {
        return;   
    }
}

void epoll_mod_out(int clientfd) {
    struct epoll_event new_epoll_event;
    memset(&new_epoll_event, 0, sizeof(new_epoll_event));
    new_epoll_event.events = EPOLLOUT;
    new_epoll_event.data.fd = clientfd;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, clientfd, &new_epoll_event);
}


void parse_header(int clientfd, client_info* client_info) {
    size_t total_amount_read = 0;
    size_t limit = 1024;
    while (total_amount_read < limit) {
        ssize_t bytes_rd = read(clientfd, client_info->header + total_amount_read, 1);
        if (client_info->header[strlen(client_info->header) - 1] == '\n') {//reach the end of the header 
            break;
        } else if (bytes_rd == -1 && errno == EINTR) {
            continue;
        } else if (bytes_rd > 0) {
            total_amount_read += bytes_rd;
        }else {
            client_info->state = -1;
            break;
        }
    }
    if (total_amount_read >= limit) {
        client_info->state = -1;
    }
    if (client_info->state == -1) {
        print_invalid_response();
        epoll_mod_out(clientfd);
        return;
    }
    char* file_name = client_info->filename;
    char* client_header = client_info->header;
    if (!strncmp("LIST\n", client_header, strlen("LIST\n"))) {
        client_info->method = LIST;
    } else if (!strncmp("GET ", client_header, strlen("GET "))) {
        client_info->method = GET;
        strcpy(file_name, client_header + strlen("GET "));
        file_name[strlen(file_name) - 1] = '\0';
    } else if (!strncmp("DELETE ", client_header, strlen("DELETE "))) {
        client_info->method = DELETE;
        strcpy(file_name, client_header + strlen("DELETE "));
        file_name[strlen(file_name) - 1] = '\0';
    } else if (!strncmp("PUT ", client_header, strlen("PUT "))) {

        client_info->method = PUT;
        strcpy(file_name, client_header + strlen("PUT "));
        file_name[strlen(file_name) - 1] = '\0';

        size_t path_length = strlen(temp_dir) + strlen(file_name) + 2; //space and \n
        char path[path_length];
        memset(path, 0, path_length);
        sprintf(path, "%s/%s", temp_dir, file_name);
        int f_ok = 0;
        if (access(path, F_OK) == 0) {
            f_ok = 1;
        }
        FILE* file = fopen(path, "w+");
        size_t file_size;
        read_all_from_socket(clientfd, (char*)&file_size, sizeof(file_size));
        size_t total_rd = 0;
        size_t block_S = 512;
        char buffer[block_S];
        while (total_rd < file_size + 5) {
            if (file_size + 5 - total_rd < block_S) {
                block_S = file_size + 5 - total_rd;
            }
            size_t bytes_rd = read_all_from_socket(clientfd, buffer, block_S);
            if (bytes_rd == 0) {
                break;
            }
            fwrite(buffer, sizeof(char), bytes_rd, file);
            total_rd += bytes_rd;
        }
        fclose(file);
        if (total_rd != file_size) {
            unlink(path);
            client_info->state = -2;
            epoll_mod_out(clientfd);
            return;
        }
        if (!f_ok) {//if file does not exist, add new one to the vec
            vector_push_back(file_name_list, file_name);
            filename_size += strlen(file_name);
        }
        dictionary_set(filename_to_filesize, file_name, &file_size);
    } else {
        print_invalid_response();
        client_info->state = -1;
        epoll_mod_out(clientfd);
        return;
    }
    client_info->state = 1;
    epoll_mod_out(clientfd);
}

int list_hldr(int clientfd) {
    write_all_to_socket(clientfd, "OK\n", 3);
    size_t size = vector_size(file_name_list) + filename_size;

    if (vector_size(file_name_list) > 0) {
        size--;
    }
    
    write_all_to_socket(clientfd, (char*)&size, sizeof(size));
    for (size_t i = 0; i < vector_size(file_name_list); i++) {
        char* curr_filename = vector_get(file_name_list, i);
        write_all_to_socket(clientfd, curr_filename, strlen(curr_filename));
        if (i != vector_size(file_name_list) - 1) {
            write_all_to_socket(clientfd, "\n", 1);
        }
    }
    return 0;
}

int get_hdlr(int clientfd, client_info* client_info) {
    char* file_name = client_info->filename;
    size_t block_S = 512;
    size_t path_length = strlen(temp_dir) + strlen(file_name) + 2;
    char path[path_length];
    memset(path, 0, path_length);
    sprintf(path, "%s/%s", temp_dir, file_name);
    FILE* file = fopen(path, "r");
    if (!file) {
        client_info->state = -3;
        epoll_mod_out(clientfd);
        return -1;
    }
    write_all_to_socket(clientfd, "OK\n", 3);
    size_t file_size = *(size_t*)dictionary_get(filename_to_filesize, file_name);
    write_all_to_socket(clientfd, (char*)&file_size, sizeof(file_size));
    size_t total_bytes_wr = 0;

    char buffer[block_S];
    while (total_bytes_wr < file_size) {
        if (file_size - total_bytes_wr < block_S) {
            block_S = file_size - total_bytes_wr;
        }
        size_t bytes_rd = fread(buffer, sizeof(char), block_S, file);
        if (bytes_rd == 0) break;
        if (write_all_to_socket(clientfd, buffer, bytes_rd) == -1) {
            client_info->state = -3;
            epoll_mod_out(clientfd);
            return -1;
        }
        total_bytes_wr += bytes_rd;
    }
    fclose(file);
    return 0;
}

int delete_hdlr(int clientfd, client_info* client_info) {
    char * file_name = client_info->filename;
    size_t path_length = strlen(temp_dir) + strlen(file_name) + 2;
    char path[path_length];
    memset(path, 0, path_length);
    sprintf(path, "%s/%s", temp_dir, file_name);
    if (access(path, F_OK) != 0) {
        client_info->state = -3;
        epoll_mod_out(clientfd);
        return -1;
    }
    unlink(path);
    for (size_t i = 0; i < vector_size(file_name_list); i++) {
        if (!strcmp(vector_get(file_name_list, i), file_name)) {
            filename_size -= strlen(file_name);
            dictionary_remove(filename_to_filesize, file_name);
            vector_erase(file_name_list, i);
            write_all_to_socket(clientfd, "OK\n", 3);
            return 0;
        }
    }
    client_info->state = -3;
    epoll_mod_out(clientfd);
    return -1;
}


int response_client(int clientfd) {
    client_info* c_info = dictionary_get(clientfd_to_clientinfo, &clientfd);
    int state = c_info->state;
    if(state == 0) {
        parse_header(clientfd, c_info);
        return 0;
    } else if (state == 1) {
        if (c_info->method == PUT) {
            write_all_to_socket(clientfd, "OK\n", 3);
        } else if (c_info->method == DELETE) {
            if (delete_hdlr(clientfd, c_info) == -1) {
                return 0;
            }
        } else if (c_info->method == GET) {
            if (get_hdlr(clientfd, c_info) == -1) {
                return 0;
            }
        } else if (c_info->method == LIST) {
            if (list_hldr(clientfd) == -1) {
                return 0;
            }
        }
    } else if (state == -1)  {
        write_all_to_socket(clientfd, "ERROR\n", 6);
        write_all_to_socket(clientfd, err_bad_request, strlen(err_bad_request));
    } else if (state == -2) {
        write_all_to_socket(clientfd, "ERROR\n", 6);
        write_all_to_socket(clientfd, err_bad_file_size, strlen(err_bad_file_size));
    } else if (state == -3) {
        write_all_to_socket(clientfd, "ERROR\n", 6);
        write_all_to_socket(clientfd, err_no_such_file, strlen(err_no_such_file));
    }
    free(c_info);
    return 1;


}


void close_server() {
    close(epoll_fd);
    //clean all the cilent_info
    vector* info_vector = dictionary_values(clientfd_to_clientinfo);
    size_t info_vector_size = vector_size(info_vector);
    for (size_t i = 0; i < info_vector_size; i++) {
        client_info* client_info = vector_get(info_vector, i);
        free(client_info);
    }
    vector_destroy(info_vector);
    //clean all the file_name
    size_t filename_vector_size = vector_size(file_name_list);
    for (size_t i = 0; i < filename_vector_size; i++) {
        char* curr_filename = vector_get(file_name_list, i);
        size_t path_length = strlen(temp_dir) + strlen(curr_filename) + 2;
        char path[path_length];
        memset(path, 0, path_length);
        sprintf(path, "%s/%s", temp_dir, curr_filename);
        unlink(path);
    }
    dictionary_destroy(filename_to_filesize);
    dictionary_destroy(clientfd_to_clientinfo);
    vector_destroy(file_name_list);
    rmdir(temp_dir);
    exit(1);
}