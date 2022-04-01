#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/*
使用管道在父子进程间传递信息
*/

int main(int argc, char *argv[]) {
    int pid;
    int pipe1[2];
    int pipe2[2];

    int x;

    if (pipe(pipe1) < 0) {
        perror("Pipe1 Create Failed.\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe2) < 0) {
        perror("Pipe2 Create Failed.\n");
        exit(EXIT_FAILURE);
    }

    if ((pid = fork()) < 0) {
        perror("Process Create Failed.\n");
        exit(EXIT_FAILURE);
    } else {
        if (pid == 0) {
            // child process
            // read from '0' of pipe1
            // write to '1' of pipe2
            // close '1' of pipe1
            // close '0' of pipe2
            close(pipe1[1]);
            close(pipe2[0]);
            // write and read loop
            do {
                read(pipe1[0], &x, sizeof(int));
                printf("Child %d read: %d\n", getpid(), x++);
                write(pipe2[1], &x, sizeof(int));
            } while (x <= 9);
            // close all pipes
            close(pipe1[0]);
            close(pipe2[1]);
            // complete child process
            exit(EXIT_SUCCESS);
        } else {
            // parent process
            close(pipe1[0]);
            close(pipe2[1]);
            // initial x
            x = 1;
            do {
                write(pipe1[1], &x, sizeof(int));
                read(pipe2[0], &x, sizeof(int));
                printf("Parent %d read: %d\n", getpid(), x++);
            } while (x <= 9);

            close(pipe1[1]);
            close(pipe2[0]);
        }
    }

    return EXIT_SUCCESS;
}