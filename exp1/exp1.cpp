#include "exp1.h"

int main(int argc, char *argv[]) {
    int pidOfLs;    // 子进程1--ls
    int pidOfPs;    // 子进程号2--ps
    int status;     // 子进程返回状态
    char *args[] = {"/bin/ls", "-a", NULL};

    // 注册函数sigcat用于处理键盘中断
    signal(SIGINT, (sighandler_t)sigcat);
    perror("SIGINT");
    // 循环四次
    int loop = 4;

    while (loop--) {
        pidOfLs = fork();
        if (pidOfLs < 0) {
            // 建立子进程失败
            printf("Create Process fail!\n");
            exit(EXIT_FAILURE);
        }
        else if (pidOfLs == 0) {
            // 子进程
            printf("=======================Child=Process=========================\n");
            printf("I am 'ls' Process %d\nMy father is %d\n", getpid(), getppid());
            pause();
            // 子进程被中断信号唤醒
            printf("%d 'ls' child will Running:\n", getpid());
            // 输出要执行的命令
            for (int i = 0; args[i] != NULL; i++)
                printf("%s\n", args[i]);
            // 执行命令行
            status = execve(args[0], &args[1], NULL);
        } else {
            sleep(1);
            // 父进程
            printf("=======================Parent=Process==========================\n");
            printf("\nI am Parent Process %d\n", getpid());
            // 子进程2--ps
            args[0] = "/bin/ps";
            args[1] = "-l";
            pidOfPs = fork();
            if (pidOfPs < 0) {
                // 创建子进程失败
                printf("Create Process fail!\n");
                exit(EXIT_FAILURE);
            } else if (pidOfPs == 0) {
                // 子进程
                printf("=======================Child=Process=========================\n");
                printf("I am 'ps' Child Process %d\nMy father is %d\n", getpid(), getppid());
                pause();
                // 子进程被中断信号唤醒
                printf("%d 'ps' child will Running\n", getpid());
                // 输出要执行的命令
                for (int i = 0; args[i] != NULL; i++)
                    printf("%s\n", args[i]);
                // 执行命令行
                status = execve(args[0], &args[1], NULL);
            } else {
                sleep(1);
                // 父进程
                printf("=======================Parent=Process==========================\n");
                printf("\nI am Parent Process %d\n", getpid());
                if(kill(pidOfPs, SIGINT) >= 0)
                    printf("%d wakeup %d child: 'ps'.\n", getpid(), pidOfPs);
                printf("%d Waiting for 'ps' child done.\n\n", getpid());
                waitpid(pidOfPs, &status, 0);
                printf("\nChild of 'ps' done, status = %d\n\n", status);
                args[0] = "/bin/ls";
                args[1] = "-a";
            }
            if(kill(pidOfLs, SIGINT) >= 0)
                printf("%d wakeup %d child: 'ls'.\n", getpid(), pidOfLs);
            printf("%d Waiting for 'ls' child done.\n\n", getpid());
            waitpid(pidOfLs, &status, 0);
            printf("\nChild of 'ls' done, status = %d\n\n", status);
        }
        sleep(3);
    }
}