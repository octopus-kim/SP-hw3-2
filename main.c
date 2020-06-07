/*
 * Year     : 2020
 * Subject  : SystemProgramming
 * Homework : Simple shell program
 * StudentID: B511032
 * Name     : SungJo Kim
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int MAXSIZE = 64;

int simple_shell(char **cmd, int count)
{
    int i, j, k, temp;
    int child_pid, status;
    int bg_flag, input_flag;
    int fd_pipe[2], fd_err_pipe[2];

    pipe(fd_pipe);
    pipe(fd_err_pipe);

    bg_flag = 0;
    if (strcmp(cmd[count - 1], "&") == 0) { bg_flag = 1; count--; }

    char *arr[count + 1];
    arr[count] = NULL;
    for (i = 0; i < count; i++) {
        arr[i] = cmd[i];
    }

    if ((child_pid = fork()) < 0) {
        fprintf(stderr, "fork() failed in argument (none)\n"); return -1;
    }

    if (child_pid == 0) {
        close(1); dup(fd_pipe[1]); close(fd_pipe[0]); close(fd_pipe[1]);
        close(2); dup(fd_err_pipe[1]); close(fd_err_pipe[0]); close(fd_err_pipe[1]);

        execvp(arr[0], arr);
        fprintf(stderr, "exec() failed in argument (none)\n"); return -1;
    } else {
        if (bg_flag == 0)
            waitpid(child_pid, &status, 0);
        else if (bg_flag == 1)
            waitpid(child_pid, &status, WNOHANG);
    }

    if ((child_pid = fork()) < 0) {
        fprintf(stderr, "fork() failed in argument (none)\n"); return -1;
    }

    if (child_pid == 0) {
        close(0); dup(fd_pipe[0]); close(fd_pipe[0]); close(fd_pipe[1]);

        exevvp("cat", NULL);
        fprintf(stderr, "exec() failed in argument (none)\n"); return -1;
    } else {
        if (bg_flag == 0)
            waitpid(child_pid, &status, 0);
        else if (bg_flag == 1)
            waitpid(child_pid, &status, WNOHANG);
    }
    /*
    i = j = k = temp = 0;
    while (i < count) {
        input_flag = 0;
        for ( ; j < count; j++) {
            if (strcmp(cmd[j], "<") == 0) {
                input_flag = 1; break;
            }
            else if (strcmp(cmd[j], "|") == 0 || strcmp(cmd[j], ">") == 0 ||
                     strcmp(cmd[j], ";") == 0 || strcmp(cmd[j], "2>") == 0) break;
        }

        if (j == count) {

        }

        if (input_flag == 1) {

        } else {

        }
    } */
}

int main(int argc, char *argv[])
{
    int cmd_len, count, i;
    char s_ptr[MAXSIZE];
    char *d_ptr;
    char **cmd;

    if (argc == 1) {
        while(1) {
            // Block process until an I/O event occurs
            printf("$ ");
            fgets(s_ptr, MAXSIZE, stdin);

            // If EOF event occurs, then exit process normally
            if (feof(stdin)) {
                printf("CTRL + D\nexit program normally\n"); return 0;
            }

            // Count input argument's number
            cmd_len = strlen(s_ptr);
            count = 1;
            for (i = 0; i < cmd_len; i++) {
                if (s_ptr[i] == ' ') count++;
            }

            // Dynamic allocation of cmd array for saving arguments
            cmd = (char**)malloc(sizeof(char*) * count);
            for (i = 0; i < count; i++)
                cmd[i] = (char*)malloc(sizeof(char) * MAXSIZE);

            // Save arguments in cmd array
            i = 0;
            d_ptr = strtok(s_ptr, " ");
            while (d_ptr != NULL) {
                strcpy(cmd[i], d_ptr);
                d_ptr = strtok(NULL, " ");
                i++;
            } i = 0;
            while (cmd[count - 1][i] != '\n') i++;
            cmd[count - 1][i] = '\0';

            if (simple_shell(cmd, count) < 0) {
                return -1;
            }
        }
    }

    else if (argc == 3) {
        // IF 2rd argument is not "-c", then exit program abnormally with error msg
        if (strcmp(argv[1], "-c") != 0) {
            printf("Usage : %s [-c] [commend] [&]\n", argv[0]); return -1;
        }

        // Count argument's number
        cmd_len = strlen(argv[2]);
        count = 1;
        for (i = 0; i < cmd_len; i++) {
            if (argv[2][i] == ' ') count++;
        }

        // Dynamic allocation of cmd array for saving arguments
        cmd = (char**)malloc(sizeof(char*) * count);
        for (i = 0; i < count; i++)
            cmd[i] = (char*)malloc(sizeof(char) * MAXSIZE);

        // Save arguments in cmd array
        i = 0;
        d_ptr = strtok(argv[2], " ");
        while (d_ptr != NULL) {
            strcpy(cmd[i], d_ptr);
            d_ptr = strtok(NULL, " ");
            i++;
        }

        if (simple_shell(cmd, count) < 0) {
            return -1;
        }
        return 0;
    }

    else {
        printf("Usage : %s [-c] [commend] [&]\n", argv[0]); return -1;
    }
}