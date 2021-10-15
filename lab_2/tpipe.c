#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

/*
使用管道在线程间传递信息
*/

static void task1(int *);
static void task2(int *);
// 两个无名管道标号
int pipe1[2], pipe2[2];
pthread_t thrd1, thrd2;

int main() {
    int ret;
    int num1, num2;

    // pipe()系统调用出两个无名线程
    if (pipe(pipe1) < 0) {
        perror("Pipe1 Create Failed.\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe2) < 0) {
        perror("Pipe2 Create Failed.\n");
        exit(EXIT_FAILURE);
    }

    // pthread_create建立线程
    num1 = 1;
    ret = pthread_create(&thrd1, NULL, (void *)task1, (void*) &num1);
    if (ret) {
        perror("Pthread Create: task1\n");
        exit(EXIT_FAILURE);
    }

    num2 = 2;
    ret = pthread_create(&thrd2, NULL, (void *)task2, (void *) &num2);
    if (ret) {
        perror("Pthread Create: task2\n");
        exit(EXIT_FAILURE);
    }
    // 执行线程thrd2, thrd1
    pthread_join(thrd2, NULL);
    pthread_join(thrd1, NULL);

    exit(EXIT_SUCCESS);
}

void task1(int *num) {
    int x = 1;
    // 每次循环向管道1的1端写入变量X, 并从管道2
    // 的0端读一个整数写入X再对X加1, 直到X > 10
    do {
        write(pipe1[1], &x, sizeof(int));
        read(pipe2[0], &x, sizeof(int));
        printf("thread%d read: %d\n", *num, x++);
    } while (x <= 9);

    close(pipe1[1]);
    close(pipe2[0]);
}

void task2(int *num) {
    int x;

    do {
        read(pipe1[0], &x, sizeof(int));
        printf("thread%d read: %d\n", *num, x++);
        write(pipe2[1], &x, sizeof(int));
    } while (x <= 9);

    close(pipe1[0]);
    close(pipe2[1]);
}