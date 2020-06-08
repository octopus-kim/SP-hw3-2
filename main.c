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
    int fdi, fdo, fde, fdt;
    int child_pid, status;
    int bg_flag, inp_flag, err_flag, out_flag, console_flag;
    int fd_temp[2];

    if ((fdt = open("../C6C86208EF", O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0600)) < 0) {
        fprintf(stderr, "ERROR open() to write\n"); return -1;
    }

    bg_flag = 0;
    if (strcmp(cmd[count - 1], "&") == 0) { bg_flag = 1; count--; }

    i = j = k = temp = 0;
    while (i < count) {
        inp_flag = err_flag = out_flag = 0;
        for (j = i; j < count; j++) {
            if (strcmp(cmd[j], "<") == 0) {
                inp_flag = 1; continue;
            }
            else if (strcmp(cmd[j], ">") == 0) {
                console_flag = 0; break;
            }
            else if (strcmp(cmd[j], "2>") == 0) {
                err_flag = 1; continue;
            }
            else if (strcmp(cmd[j], "|") == 0) {
                console_flag = 0; break;
            }
            else if (strcmp(cmd[j], ";") == 0) {
                console_flag = 1; break;
            }
            console_flag = 1;
        }

        if (inp_flag == 0 && err_flag == 0)
            temp = j - i;
        else if (inp_flag == 0 && err_flag == 1) {
            temp = j - i - 2;
            if ((fde = open(cmd[j - 1], O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0600)) < 0) {
                fprintf(stderr, "ERROR open() to write: %s\n", cmd[j - 1]); return -1;
            }
        }
        else if (inp_flag == 1 && err_flag == 0) {
            temp = j - i - 2;
            if ((fdi = open(cmd[j - 1], O_RDONLY | O_NONBLOCK)) < 0) {
                fprintf(stderr, "ERROR open() to read: %s\n", cmd[j - 1]); return -1;
            }
        }
        else if (inp_flag == 1 && err_flag == 1) {
            temp = j - i - 4;
            if ((fdi = open(cmd[j - 3], O_RDONLY | O_NONBLOCK)) < 0) {
                fprintf(stderr, "ERROR open() to read: %s\n", cmd[j - 3]); return -1;
            }
            if ((fde = open(cmd[j - 1], O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0600)) < 0) {
                fprintf(stderr, "ERROR open() to write: %s\n", cmd[j - 1]); return -1;
            }
        }

        if (j < count) {
            if (strcmp(cmd[j], ">") == 0) {
                if ((fdo = open(cmd[j + 1], O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0600)) < 0) {
                    fprintf(stderr, "ERROR open() to write: %s\n", cmd[j + 1]); return -1;
                } j += 2;
                out_flag = 1;
                if (j < count) {
                    if (strcmp(cmd[j], "2>") == 0) {
                        if ((fde = open(cmd[j + 1], O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0600)) < 0) {
                            fprintf(stderr, "ERROR open() to write: %s\n", cmd[j + 1]); return -1;
                        } j += 2;
                        err_flag = 1;
                    }
                }
            }
        }

        char* arr[temp + 1];
        arr[temp] = NULL;
        for (k = 0; k < temp; k++) {
            arr[k] = cmd[i]; i++;
        }

        if ((child_pid = fork()) < 0) {
            fprintf(stderr, "ERROR fork()\n"); return -1;
        }

        if (child_pid == 0) {
            if (inp_flag == 1) {
                close(0); dup(fdi); close(fdi);
            }
            if (console_flag == 0) {
                if (out_flag == 1) {
                    close(1); dup(fdo); close(fdo);
                }
                else {
                    close(1); dup(fdt); close(fdt);
                }
            }
            if (err_flag == 1) {
                close(2); dup(fde); close(fde);
            }

            execvp(arr[0], arr);
            fprintf(stderr, "ERROR exec()\n"); return -1;
        } else {
            if (bg_flag == 0)
                waitpid(child_pid, &status, 0);
            else if (bg_flag == 1)
                waitpid(child_pid, &status, WNOHANG);
        } i = j;


        while (i < count) {
            if (strcmp(cmd[j], "|") == 0) {
                i++;
                console_flag = err_flag = out_flag = 0;
                for (j = i; j < count; j++) {
                    if (strcmp(cmd[j], ">") == 0) {
                        console_flag = 0; break;
                    }
                    else if (strcmp(cmd[j], "2>") == 0) {
                        err_flag = 1; continue;
                    }
                    else if (strcmp(cmd[j], "|") == 0) {
                        console_flag = 2; break;
                    }
                    else if (strcmp(cmd[j], ";") == 0) {
                        console_flag = 1; break;
                    }
                    console_flag = 1;
                }

                if (err_flag == 1) {
                    temp = j - i - 2;
                    if ((fde = open(cmd[j - 1], O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0600)) < 0) {
                        fprintf(stderr, "ERROR open() to write: %s\n", cmd[j - 1]); return -1;
                    }
                } else temp = j - i;

                char *arr[temp + 1];
                arr[temp] = NULL;
                for (k = 0; k < temp; k++) {
                    arr[k] = cmd[i]; i++;
                }

                if (console_flag == 2) {
                    pipe(fd_temp);
                    if ((child_pid = fork()) < 0) {
                        fprintf(stderr, "ERROR fork()\n"); return -1;
                    }

                    if (child_pid == 0) {
                        if (err_flag == 1) {
                            close(2); dup(fde); close(fde);
                        }
                        close(0); dup(fdt); close(fdt);
                        close(1); dup(fd_temp[0]); close(fd_temp[0]); close(fd_temp[1]);

                        execvp(arr[0], arr);
                        fprintf(stderr, "ERROR exec()\n"); return -1;
                    } else {
                        if (bg_flag == 0)
                            waitpid(child_pid, &status, 0);
                        else if (bg_flag == 1)
                            waitpid(child_pid, &status, WNOHANG);
                    }

                    if ((child_pid = fork()) < 0) {
                        fprintf(stderr, "ERROR fork()\n"); return -1;
                    }

                    if (child_pid == 0) {
                        close(0); dup(fd_temp[0]); close(fd_temp[0]); close(fd_temp[1]);
                        close(1); dup(fdt); close(fdt);

                        execlp("cat", "cat", 0);
                        fprintf(stderr, "ERROR exec()\n"); return -1;
                    } else {
                        if (bg_flag == 0) {
                            close(fd_temp[0]); close(fd_temp[1]);
                            waitpid(child_pid, &status, 0);
                        }
                        else if (bg_flag == 1) {
                            close(fd_temp[0]); close(fd_temp[1]);
                            waitpid(child_pid, &status, WNOHANG);
                        }
                    }
                }

                else {
                    if (j < count) {
                        if (strcmp(cmd[j], ">") == 0) {
                            if ((fdo = open(cmd[j + 1], O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0600)) < 0) {
                                fprintf(stderr, "ERROR open() to write: %s\n", cmd[j + 1]); return -1;
                            } j += 2;
                            out_flag = 1;
                            if (j < count) {
                                if (strcmp(cmd[j], "2>") == 0) {
                                    if ((fde = open(cmd[j + 1], O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0600)) < 0) {
                                        fprintf(stderr, "ERROR open() to write: %s\n", cmd[j + 1]); return -1;
                                    } j += 2;
                                    err_flag = 1;
                                }
                            }
                        }
                    }

                    if ((child_pid = fork()) < 0) {
                        fprintf(stderr, "ERROR fork()\n"); return -1;
                    }

                    if (child_pid == 0) {
                        if (console_flag == 0) {
                            if (out_flag == 1) {
                                close(1); dup(fdo); close(fdo);
                            }
                        }
                        if (err_flag == 1) {
                            close(2); dup(fde); close(fde);
                        }
                        close(0); dup(fdt); close(fdt);

                        execvp(arr[0], arr);
                        fprintf(stderr, "ERROR exec()\n"); return -1;
                    } else {
                        if (bg_flag == 0) {
                            waitpid(child_pid, &status, 0);
                        }
                        else if (bg_flag == 1) {
                            waitpid(child_pid, &status, WNOHANG);
                        }
                    } j = i;
                }
            }

            else if (strcmp(cmd[j], ";") == 0) {
                i += 1; j += 1; break;
            }
        }
    } while (remove("../C6C86208EF") < 0) close(fdt);
}

int main(int argc, char *argv[])
{
    int  cmd_len, count, i;
    char s_ptr[MAXSIZE];
    char *d_ptr;
    char **cmd;

    if (argc == 1) {
        while (1) {
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

            if (simple_shell(cmd, count) < 0) continue;
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