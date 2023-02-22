/**
 * utilities_unleashed
 * CS 341 - Fall 2022
 */
 #include <stdio.h>
 #include <stdint.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <sys/types.h>
 #include <sys/wait.h>
 #include <string.h>
 #include <ctype.h>
 #include "format.h"
int main(int argc, char *argv[]) {
    int idx = 0;
    int find_dash = 0;
    while ( argv[idx] != NULL) {
        if(!strcmp(argv[idx], "--")) {
            find_dash = 1;
        }
        idx++;
    }
    if(!find_dash) {
        
        print_env_usage();
    }

  pid_t child_id = fork();
  if (child_id < 0) { 
      print_fork_failed();
  } else if(child_id > 0){ 
      int status;
      waitpid(child_id, &status, 0);
  }  else if(child_id == 0){
        int i = 1;

        while ( argv[i] != NULL) {
          if (strcmp(argv[i], "--")) {
                char *key = strtok(argv[i], "=");
                char *value = strtok(NULL, "");
                if (value == NULL || key == NULL) {
                    print_env_usage();
                }
                char *p_key = key;
                while (*p_key) {
                    if ( !isalnum(*p_key) || *p_key == '_')  {
                        
                        print_env_usage();
                    }
                    p_key++;
                }
                if (value[0] != '%') {
                    char *p_value = value;
                    while (*p_value) {
                        if ( !(isalnum(*p_value) || *p_value == '_'))  {
                            
                            print_env_usage();
                        }
                        p_value++;
                    }
                } else if(value[0] == '%'){
                    value = value + 1;
                    value = getenv(value);
                    if (!value) {
                        print_environment_change_failed();
                    }
                }
                int success_ = setenv(key, value, 1);
                if (success_ < 0 ){
                    print_environment_change_failed();
                }
          } else {
            if(argv[i+1] == NULL ) {
                
                print_env_usage();
            }
                    execvp(argv[i+1], argv + i + 1);
                    print_exec_failed();
          }
          i++;
        }
        print_env_usage();
  }
  return 0;
}
