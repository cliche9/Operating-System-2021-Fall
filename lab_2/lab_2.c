#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// write: pipe[1]
// read: pipe[0]
int pipe_x[2];
int pipe_y[2];

typedef struct {
    int x;
    int y;
} pair;

static void fx(int *x) {
    int i;
    int res = 1;
    for (i = 1; i <= *x; i++)
        res *= i;
    write(pipe_x[1], &res, sizeof(int));
    printf("===================Thread f(x)====================\n");
    printf("f(x) write %d to pipe_x[0]\n", res);
    close(pipe_x[1]);
}

static int solve(int y) {
    if (y == 1 || y == 2)
        return 1;
    return solve(y - 1) + solve(y - 2);
}

static void fy(int *y) {
    int res = solve(*y);
    write(pipe_y[1], &res, sizeof(int));
    printf("===================Thread f(y)====================\n");
    printf("f(y) write %d to pipe_y[0]\n", res);
    close(pipe_y[1]);
}

static void fxy(int *x, int *y) {
    int fx = 0, fy = 0, res = 0;
    read(pipe_x[0], &fx, sizeof(int));
    printf("===================Thread f(x, y)====================\n");
    printf("f(x, y) read %d from pipe_x[1]\n", fx);
    read(pipe_y[0], &fy, sizeof(int));
    printf("===================Thread f(x, y)====================\n");
    printf("f(x, y) read %d from pipe_y[1]\n", fy);
    res = fx + fy;
    printf("f(x, y) = %d\n", res);
    close(pipe_x[0]);
    close(pipe_y[0]);
}

int main() {
    int x = 0, y = 0, status = 0;
    pair xy;
    pthread_t fx_tid, fy_tid, fxy_tid;

    if (pipe(pipe_x) < 0) {
        perror("Pipe_x Create Failed.\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_y) < 0) {
        perror("Pipe_y Create Failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("x = ");
    scanf("%d", &x);
    printf("y = ");
    scanf("%d", &y);
    xy.x = x;
    xy.y = y; 
    status = pthread_create(&fx_tid, NULL, (void *)fx, (void *) &x);
    if (status) {
        perror("Thread f(x) Create Failed.\n");
        exit(EXIT_FAILURE);
    }
    status = pthread_create(&fy_tid, NULL, (void *)fy, (void *) &y);
    if (status) {
        perror("Thread f(y) Create Failed.\n");
        exit(EXIT_FAILURE);
    }
    status = pthread_create(&fxy_tid, NULL, (void *)fxy, (void *) &xy);
    if (status) {
        perror("Thread f(x, y) Create Failed.\n");
        exit(EXIT_FAILURE);
    }

    pthread_join(fx_tid, NULL);
    pthread_join(fy_tid, NULL);
    pthread_join(fxy_tid, NULL);

    exit(EXIT_SUCCESS);
}