#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_SIZE 100
#define MAX_WAITINGS 20

void str_dup(char **arr, int n, char *line, ssize_t nread){
    arr[n] = strndup(line, nread);
    if(arr[n] == NULL){
        perror("strndup failed\n");
        exit(EXIT_FAILURE);
    }
}

void printf_t(char** arr, int n){
    for (int i = 0; i <= n; i++){
        fprintf(stderr, "%s ", arr[i]);
    }
    fprintf(stderr, "\n");
}

int wait_limit(){
    int waiting_count = 0, w;

    // if waitpid failes once, we try again a number of times. 
    // If it also failes after that, we report the error.
    while(waiting_count < MAX_WAITINGS){ 
        if((w = waitpid(-1, NULL, 0)) != -1){
            break;
        }
        waiting_count++;
    }

    if(waiting_count == MAX_WAITINGS){
        perror("waitpid failed too many times\n");
        return EXIT_FAILURE;
    }
    printf("/////// process id returned: %d ////////////\n", w);
    return w;
}

int main(int argc, char **argv){
    int opt, t = 0, current_index, arg_count, max_args = -1, initial_index, max_processes = 1, processes_count = 0, p;
    char *line;
    char *inputArr[MAX_SIZE] = {NULL};
    size_t len = 0;
    ssize_t nread;
    pid_t f;

    while((opt = getopt(argc, argv, "tn:j:")) != -1){
        switch (opt){
        case 't':
            t = 1;
            break;
        case 'n':
            max_args = atoi(optarg);
            break;
        case 'j':
            max_processes = atoi(optarg);
        default:
            break;
        }
    }

    pid_t *process_array = (pid_t*)malloc(max_processes);
    
    current_index = 0;
    //copy commands into inputArrays, if there are any. If not, set "/bin/echo" command as default
    if(argc != optind){
        for(int i = optind; i < argc; i++){
            inputArr[current_index] = argv[i];
            current_index++;
        }
    }
    else{
        inputArr[0] = "/bin/echo";
        current_index = 1;
    }

    initial_index = current_index;
    arg_count = 0;

    while((nread = getline(&line, &len, stdin)) != -1){
        if(line[nread - 1] == '\n'){
            nread--;
        }

        // inputArr[current_index] = strndup(line, nread);

        str_dup(inputArr, current_index, line, nread);

        arg_count++;
        if(current_index == MAX_SIZE - 1 || arg_count == max_args){
            inputArr[current_index + 1] = '\0';
            
            //if there is no space for a new process to run, we wait and get the id of the 
            //first process that finishes
            if(processes_count == max_processes){
               p = wait_limit(); 
            }
            f = fork();
            if(!f){
                if(t){
                    printf_t(inputArr, current_index);
                }
                execvp(inputArr[0], inputArr);
                fprintf(stderr, "process-%d failed\n", getpid());
                return EXIT_FAILURE;
            }
            if(f == -1){
                perror("fork failed\n");
            }
            else{
                if(processes_count == max_processes){
                    int i;
                    for(i = 0; i < max_processes; i++){
                        if(process_array[i] == p)
                            break;
                    }
                    process_array[i] = f;
                }
                else{
                    process_array[processes_count] = f;
                    processes_count++;
                }
                
            }
            arg_count = 0;
            current_index = initial_index;
        }
        else{
            current_index++;
        }
    }
    if(current_index != initial_index){
        inputArr[current_index] = '\0';
        f = fork();
        if(!f){
            if(t){
                printf_t(inputArr, current_index - 1);
            }
            execvp(inputArr[0], inputArr);
            fprintf(stderr, "process-%d failed\n", getpid());
            return EXIT_FAILURE;
        }
        wait_limit(f);
    }

    // free memory allocated with strndup from the initial index of storing stdin up to the maximum
    // size of teh array, if the pointer is not null
    for(int i = initial_index; i < MAX_SIZE; i++){
        if(inputArr[i] != NULL)
            free(inputArr[i]);
    }

    //free memory allocated for processes array
    free(process_array);

    return EXIT_SUCCESS;
}