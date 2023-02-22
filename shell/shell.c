/**
 * shell
 * CS 341 - Fall 2022
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "unistd.h"
#include "stdio.h"
#include "string.h"
#include "sstring.h"
#include "ctype.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>


typedef struct process {
    char *command;
    pid_t pid;
} process;

static vector* history_vec;
static vector* process_vec;
static char* full_path_h;
static int _dash_h = 0;

int excute_command(char* command, int logic_o);

process* create_pcs(const char *buffer, pid_t pid) {
  process *pcs = calloc(1, sizeof(process));
  pcs->command = calloc(strlen(buffer)+1, sizeof(char));
  strcpy(pcs->command, buffer);
  pcs->pid = pid;
  return pcs;
}
void destroy_pcs(pid_t pid) {
  for (size_t i = 0; i < vector_size(process_vec); i++) {
    process *pcs = (process *) vector_get(process_vec, i);
    if ( pcs->pid == pid ) {
      free(pcs->command);
      free(pcs);
      vector_erase(process_vec, i);
      return;
    }
  }
}

int get_history(char* path) {
    FILE* fp = NULL;
    fp = fopen(path, "r");
    if(fp == NULL) {
        return -1;
    } else {
    char* buff = NULL;
    size_t capacity = 0;
    while(getline(&buff, &capacity, fp) != -1) {
        if (buff[strlen(buff) - 1] == '\n'){
            buff[strlen(buff) - 1] = '\0';
        }   
        vector_push_back(history_vec, buff);
    }
    fclose(fp);
    return 1;
    }
}

vector* get_command_from_file(char* path) {
    FILE* fp = NULL;
    fp = fopen(path, "r");
    if (fp == NULL) {
        print_script_file_error();
    }
    vector* file_vec = string_vector_create();
    char* buff = NULL;
    size_t capacity = 0;
    ssize_t result = 0;

    while((result = getline(&buff, &capacity, fp)) != -1) {
        if (buff[strlen(buff) - 1] == '\n'){
            buff[strlen(buff) - 1] = '\0';
        }
        vector_push_back(file_vec, buff);
    }
    fclose(fp);
    free(buff);
    return file_vec;
}

void destroy_process(pid_t pid) {
        for (size_t i = 0; i < vector_size(process_vec); i++) {
        process *pcs = (process *) vector_get(process_vec, i);
        if ( pcs->pid == pid ) {
            free(pcs->command);
            free(pcs);
            vector_erase(process_vec, i);
        return;
        }
    }

}

void ctrlcpressed() {
  for (size_t i = 0; i < vector_size(process_vec); i++) {
    process *pcs = (process *) vector_get(process_vec, i);
    if ( pcs->pid != getpgid(pcs->pid) ){
      kill(pcs->pid, SIGKILL);
      destroy_process(pcs->pid);
    }
  }
}



void kill_shell(){
    for (size_t i = 0; i < vector_size(process_vec); i++) {
        process *pcs = (process *) vector_get(process_vec, i);
        kill(pcs->pid, SIGKILL);
    }
    for (size_t i = 0; i < vector_size(process_vec); i++) {
        process *pcs = (process *) vector_get(process_vec, i);
        free(pcs->command);
        free(pcs);
    }
    vector_destroy(process_vec);
}

char** str_split(char* input_string, char delim) {
    sstring* string = cstr_to_sstring(input_string);
    vector* vec = sstring_split(string,delim);

    char** char_arr = (char**)malloc(sizeof(char*) * (vector_size(vec) + 1));
    for (size_t i = 0; i < vector_size(vec); i++) {
        char_arr[i] = strdup(vector_get(vec, i));
    }
    char_arr[vector_size(vec)] = NULL;
    return char_arr;
}

process_info *process_info_create(char *command, pid_t pid) {
    process_info *pcs_info = calloc(1, sizeof(process_info));
    int cmd_len = strlen(command);
    pcs_info -> command = calloc(cmd_len + 1, sizeof(char));
    strcpy(pcs_info -> command, command);
    pcs_info -> pid = pid;
 
    char path[40], info_line[1000], *p;
    snprintf(path, 40, "/proc/%d/status", pid);
    FILE *fd = fopen(path, "r");
    if (!fd) {
        print_script_file_error();
        exit(1);
    }
    while (fgets(info_line, 100, fd)) {
        if (!strncmp(info_line, "State:", 6)) {
            p = info_line + 7;
            while (isspace(*p)){
                ++p;
            }
            pcs_info->state = *p;
        } else if (!strncmp(info_line, "Threads:", 8)) {
            char *r;
            p = info_line + 9;
            while (isspace(*p)){
                ++p;
            }
            pcs_info->nthreads = strtol(p, &r, 10);
        } else if (!strncmp(info_line, "VmSize:", 7)) {
            char *r;
            p = info_line + 8;
            while (isspace(*p)){
                ++p;
            }
            pcs_info->vsize = strtol(p, &r, 10);
        }
    }

    fclose(fd);
    snprintf(path, 40, "/proc/%d/stat", pid);
    FILE *fp_f = fopen(path, "r");
    if (!fp_f) {
        print_script_file_error();
        exit(1);
    }

    fgets(info_line, 1000, fp_f);
    fclose(fp_f);
    p = strtok(info_line, " ");
    int i = 0;
    char *cpu_time;
    unsigned long utime, stime;
    unsigned long long start_time;
    while (p != NULL) {
        if (i == 13) {
            utime = strtol(p, &cpu_time, 10);
        } else if (i == 14) {
            stime = strtol(p, &cpu_time, 10);
        } else if (i == 21) {
            start_time = strtol(p, &cpu_time, 10);
        }
        p = strtok(NULL, " ");
        i++;
    }
    char buffer_cpu[100];
    memset(buffer_cpu, 0, 100);
    unsigned long total_seconds_cpu = (utime + stime) / sysconf(_SC_CLK_TCK);
    if (!execution_time_to_string(buffer_cpu, 100, total_seconds_cpu / 60, total_seconds_cpu % 60)){
        exit(1);
    }

    pcs_info->time_str = calloc(strlen(buffer_cpu) + 1, sizeof(char));
    strcpy(pcs_info->time_str, buffer_cpu);
    FILE *stat_fd = fopen("/proc/stat", "r");
    if (!stat_fd) {
        print_script_file_error();
        exit(1);
    }

    unsigned long btime;
    while (fgets(info_line, 100, stat_fd)) {
        if (!strncmp(info_line, "btime", 5)) {
        p = info_line + 6;
        while (isspace(*p)){
            ++p;
        }
        btime = strtol(p, &cpu_time, 10);
        }
    }
    fclose(stat_fd);
    char buffer[100];
    memset(buffer, 0, 100);
    time_t second_total = start_time / sysconf(_SC_CLK_TCK) + btime;
    struct tm *tm_ = localtime(&second_total);
    if (!time_struct_to_string(buffer, 100, tm_)){
        exit(1);
    }
    pcs_info->start_str = calloc(strlen(buffer) + 1, sizeof(char));
    strcpy(pcs_info->start_str, buffer);
    return pcs_info;
}

int builtin_command(char* buff) {
    //Command cd
    //buff: "cd cs341/shell"
    //demited by 空格
    char** command_arr = str_split(buff, ' ');
    //退出 好像能用

    if(strcmp(command_arr[0], "exit") == 0) {
        //invalid free
        exit(0);
    }

    if (strcmp(command_arr[0], "cd") == 0) {
      vector_push_back(history_vec,buff);
      int sucess = chdir(command_arr[1]);
      if (sucess < 0) {
        print_no_directory(command_arr[1]);
        return 0;
      } 
        return 1;
    } 

    if (strcmp(buff, "!history\0") == 0) {
        
        for (size_t i = 0; i < vector_size(history_vec); i++) {
            print_history_line(i, vector_get(history_vec,i));
        }
        
        return 1;
    }
    if (buff[0] == '#') {
        sstring *whatever = cstr_to_sstring(buff);
        char* find_index = sstring_slice(whatever, 1,(int)sizeof(buff));
        if((size_t)(atoi(find_index)) + 1 >  (vector_size(history_vec))) {
            print_invalid_index();
            return 0;
        }
        print_command(vector_get(history_vec,(size_t)(atoi(find_index))));

        char* command_to_exec = vector_get(history_vec,(size_t)(atoi(find_index)));
        excute_command(command_to_exec,0);
        return 3;


    } 
    if (buff[0] == '!') {
          for (int i = vector_size(history_vec) - 1; i >= 0 ; --i) {
            char *history_cmd = (char *)vector_get(history_vec, i);
            if (buff[1] == '\0' || !strncmp(buff + 1, history_cmd, strlen(buff+1))) {
              print_command(history_cmd);
              excute_command(history_cmd,0);
              return 1;
            }
            if (i == 0) {
                print_no_history_match();
                return 0;
          }
        }
    } if (strcmp(command_arr[0], "ps") == 0) {
        print_process_info_header();
        for (size_t i = 0; i < vector_size(process_vec); i++) {
            process *p = vector_get(process_vec, i);
            process_info *p_i = process_info_create(p->command, p->pid);
            print_process_info(p_i);
        }
        return 4;
    }
    return 2;
}  


int external_command(char *buffer) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) { 
        print_fork_failed();
        exit(1);
    } else if (pid > 0) { 
        process *prcss = create_pcs(buffer, pid);
        vector_push_back(process_vec, prcss);
        if (buffer[strlen(buffer) - 1] == '&') {
            if (setpgid(pid, pid) == -1) {
                print_setpgid_failed();
                exit(1);
            }
        } else {
            if (setpgid(pid, getpgid(getpid())) < 0) {
                print_setpgid_failed();
                exit(1);
            }
            int status;
            pid_t ret_pid = waitpid(pid, &status, 0);
            if (ret_pid < 0) {
                print_wait_failed();
                exit(1);
            }      
            destroy_pcs(pid);
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            }
        }
    } else { 
        if (buffer[strlen(buffer) - 1] == '&'){
            buffer[strlen(buffer) - 1] = '\0';
        }

        vector *command_vec = sstring_split(cstr_to_sstring(buffer), ' ');
        char *command[vector_size(command_vec) + 1];

        for (size_t i = 0; i < vector_size(command_vec); i++) {

            command[i] = (char *) vector_get(command_vec, i);

        }
        if (!strcmp(command[vector_size(command_vec) - 1], "")){
            command[vector_size(command_vec) - 1] = NULL;
        } else {
            command[vector_size(command_vec)] = NULL;
        }
        int stdin_flg = 0;
        FILE* fp = NULL;
        //puts("do something");
        for (size_t i = 0; i < vector_size(command_vec) + 1; ++i) {
            //puts("do something2");
            if(command[i] == NULL) {
                break;
            }
            if (strcmp(command[i], ">") == 0) {
                //puts("do something3");
                if (command[i + 1] == NULL) {
                    print_usage();
                    exit(1);
                }
                fp = fopen(command[i + 1], "w+");
                if (fp == NULL) {
                    print_redirection_file_error();
                    exit(1);
                }
                command[i] = NULL;
                break;
            }
            //puts("do something7");
            if (strcmp(command[i], ">>") == 0) {
                //puts("do something4");
                if (command[i + 1] == NULL) {
                    print_usage();
                    exit(1);
                }
                fp = fopen(command[i + 1], "a+");
                if (fp == NULL) {
                    print_redirection_file_error();
                    exit(1);
                }
                command[i] = NULL;
                break;
            }
            //puts("do something8");
            if (strcmp(command[i], "<") == 0) {
                //puts("do something5");
                if (command[i + 1] == NULL) {
                    print_usage();
                    exit(1);
                }
                fp = fopen(command[i + 1], "r");
                if (fp == NULL) {
                    print_redirection_file_error();
                    exit(1);
                }
                command[i] = NULL;
                stdin_flg = 1;
                break;
            }
            //puts("do something end");
        }
        //puts("do something6");
        print_command_executed(getpid());
        if (fp != NULL) {
            if (stdin_flg == 1) {
                dup2(fileno(fp), STDIN_FILENO);
            } else {
                dup2(fileno(fp), STDOUT_FILENO);
            }
        fclose(fp);
        }
        execvp(command[0], command);
        print_exec_failed(command[0]);
        exit(1);
    }
        return 0;
}




int excute_command(char* command, int logic_o) {
    int status_built_in = builtin_command(command);
    if(status_built_in == 1){
        print_command_executed(getpid());
        return 1;
    }else if (status_built_in == 4) {
        return 1;
    }
    else if(!logic_o && status_built_in != 3){
        int status = external_command(command);
        vector_push_back(history_vec,command);
        return !status;
    }else if(status_built_in == 2) {
        if(!external_command(command)) {
            return 1;
        }
    }
    return 0;
}

int shell(int argc, char *argv[]) {
    signal(SIGINT, ctrlcpressed);
    process_vec = shallow_vector_create();
    history_vec = vector_create(string_copy_constructor, string_destructor, string_default_constructor);


    //int another_ch;
    char* file_to_be_handle = NULL;

    int ch;
    while((ch = getopt(argc,argv,"h:f:")) != -1) {
        switch(ch) {
            case 'h':
                full_path_h = get_full_path(optarg);
                if(get_history(full_path_h) == -1) {
                    //create a new file
                    fopen(argv[optind], "w");
                    full_path_h = get_full_path(optarg);
                }
                
                _dash_h = 1;
                break;

            case 'f':
                if(optarg ==  NULL) {
                    print_usage();
                    break;
                } 
                file_to_be_handle = get_full_path(optarg);
                break;

            default:
                break;
        }
    }


    FILE* file;
    if(file_to_be_handle) {
        file = fopen(file_to_be_handle, "r");
        if (!file) {
            print_script_file_error();
            exit(1);
        }
    } else {
        file = stdin;
    }
    
    char *buff = NULL;
    size_t size = 0;
    ssize_t bytes_num;
    while (1) {
        char prompt_print[100];
        getcwd(prompt_print, sizeof(prompt_print));
        print_prompt(getcwd(prompt_print, sizeof(prompt_print)), getpid());

        bytes_num = getline(&buff,&size, file);
        if (bytes_num == -1) {
            kill_shell();
            break;
        }
        if (bytes_num > 0 && buff[bytes_num - 1] == '\n') {
            buff[bytes_num - 1] = '\0';
        }

        if(file != stdin) {         
            print_command(buff);
        } 

        //logic operation
        int logic_o = 0;
        sstring *s_str = cstr_to_sstring(buff);
        vector *command_vec = sstring_split(s_str, ' ');
        for (size_t i = 0; i < vector_size(command_vec); i++) {
            char *str = (char *)vector_get(command_vec, i);
            if (strcmp(str, "&&") == 0) {
                vector_push_back(history_vec, buff);
                char *token1 = strtok(buff, "&");
                char *token2 = strtok(NULL, "&");
                ++token2;
                if (excute_command(token1,1)) {
                    excute_command(token2,1);
                }
                logic_o = 1;
            } else if (strcmp(str, "||") == 0) {
                vector_push_back(history_vec, buff);
                char *token1 = strtok(buff, "|");
                char *token2 = strtok(NULL, "|");
                ++token2;
                if (!excute_command(token1,1)) {
                    excute_command(token2,1);
                }
                logic_o = 1;
            } else if (strcmp(str + strlen(str) - 1, ";") == 0) {
                vector_push_back(history_vec, buff);
                char *token1 = strtok(buff, ";");
                char *token2 = strtok(NULL, ";");
                ++token2;
                excute_command(token1,1);
                excute_command(token2,1);
                logic_o = 1;
            } 
        }
        sstring_destroy(s_str);
        vector_destroy(command_vec);
        if (!logic_o) {
            excute_command(buff,0); 
        }
    if(_dash_h) {
        FILE* fp = fopen(full_path_h, "w");
        for(size_t i = 0; i < vector_size(history_vec); ++i) {
            fprintf(fp,"%s\n",vector_get(history_vec,i));
        }
        fclose(fp);    
    }


    }

    return 0;
}



