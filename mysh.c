// mysh - a simple Unix shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

int main(void) {
    char line[1024];
    char *args[64]; 
    int arg_count;
    while(1){
        printf("mysh> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        arg_count = 0; 
        args[arg_count] = strtok(line, " "); 

        while (args[arg_count] != NULL) { 
            arg_count++; 
            args[arg_count] = strtok(NULL, " "); 
        }

        if (arg_count == 0) {
            continue;
        }

        if (strcmp(args[0], "cd") == 0) {
            char *dir;

            if (arg_count < 2) {
                dir = getenv("HOME");
            } 
            else {
                dir = args[1];
            }

            if (chdir(dir) != 0) {
            perror("cd failed");
            }

            continue;
        }

        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        char *infile = NULL;
        char *outfile = NULL;

        for (int i = 0; i < arg_count; i++) {
            if (args[i] == NULL) continue;

            if (strcmp(args[i], "<") == 0) {
                if (i + 1 < arg_count) {
                    infile = args[i + 1];
                    args[i] = NULL;
                    args[i + 1] = NULL;
                }
            }
            else if (strcmp(args[i], ">") == 0) {
                if (i + 1 < arg_count) {
                    outfile = args[i + 1];
                    args[i] = NULL;
                    args[i + 1] = NULL;
                }
            }
        }

        int pipe_index = -1;

        for (int i = 0; i < arg_count; i++) {
            if (args[i] != NULL && strcmp(args[i], "|") == 0) {
                pipe_index = i;
                args[i] = NULL;
                break;
            }
        }

        if (pipe_index != -1) {
            char **left = args;
            char **right = &args[pipe_index + 1];
            int pipefd[2];
            pipe(pipefd);

            pid_t pid1 = fork();

            if (pid1 == 0) {
                close(pipefd[0]);

                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);

                execvp(left[0], left);
                perror("execvp failed");
                exit(1);
            }

            pid_t pid2 = fork();

            if (pid2 == 0) {
                close(pipefd[1]);

                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);

                execvp(right[0], right);
                perror("execvp failed");
                exit(1);
            }

            close(pipefd[0]);
            close(pipefd[1]);

            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);

            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            continue;
        }

        if (pid == 0) {

            if (infile != NULL) {
                int fd = open(infile, O_RDONLY);

                if (fd < 0) {
                    perror("open failed");
                    exit(1);
                }

                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            
            if (outfile != NULL) {
                int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
                if (fd < 0) {
                    perror("open failed");
                    exit(1);
                }

                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(args[0], args);
            perror("execvp failed");
            exit(1);
        }

        else {
            waitpid(pid, NULL, 0);
        }
    }
    return 0;
}