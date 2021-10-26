#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
// 指令长度 历史记录 路径长度 参数个数
#define EXE_LENGTH 200
#define PARAMETER_SIZE 64
#define HISTORY_SIZE 1024
#define MAX_PATH 1024
typedef void(*sighandler_t)(int);
void sigcat(pthread_t pid) {
    kill(pid, SIGINT);
}

// 1-parse user's input
int parse(char *word, char **argv) {
    int count = 0;
    // 初始化参数表, 用于存储待执行指令
    memset(argv, 0, sizeof(char*) * PARAMETER_SIZE);
    // parse过程剩余的未parse的字符串
    char *lefts = NULL;
    // 设置delimeter
    const char *split = " ";
    while (1) {
        // 分割一个参数到p
        char *p = strtok_r(word, split, &lefts); // ref-"strtok-r":http://baike.baidu.com/view/6991509.htm?fr=aladdin
        // parsing completed
        if (p == NULL)
            break;
        // argv is an array, it stores each value of your input divided by " "
        argv[count] = p;
        word = lefts;
        /* debug
        printf("Argvs: %s\n", argv[count]);
        printf("Lefts: %s\n", word);
        */
        count++;
    }
    return count;
}

//2-get the first word
void trim(char *string) {
    // 默认文件名不包含空格
    int left = 0;
    int right = strlen(string) - 1;
    if (left > right)
        return;
    char *temp = (char *)malloc(sizeof(char) * (right + 1));
    strcpy(temp, string);

    while (string[left] != '\0' && string[left] == ' ')
        left++;
    while (right > left && string[right] == ' ')
        right--;

    strncpy(string, temp + left, right - left + 1);
    free(temp);
    /*
    // debug
    printf("%s\n",string);
    printf("%d\n",j);
    */
}

static void executeWithoutPipe(int start, int end, char **argv);
static void executeWithPipe(int start, int end, char **argv);
//3-execute the basic order
void execute(int argc, char **argv) {
    pid_t pid;
    int status;
    // 创建子进程执行
    if ((pid = fork()) < 0) {
        perror("Create Process Failed.\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 记录标准输入输出
        int ifd = dup(STDIN_FILENO);
        int ofd = dup(STDOUT_FILENO);
        executeWithPipe(0, argc, argv);
        // 输入输出重定向回标准输入输出
        dup2(ifd, STDIN_FILENO);
        dup2(ofd, STDOUT_FILENO);
    } else
        while (wait(&status) != pid); //ref-"wait":http://see.xidian.edu.cn/cpp/html/289.html
}

void executeWithPipe(int start, int end, char **argv) {
    // 涉及到多个管道, 递归执行
    if (start >= end)
        return;

    int positionOfPipe = -1;
    // 寻找管道标记位置
    for (int i = start; i < end; i++) {
        if (strcmp(argv[i], "|") == 0) {
            positionOfPipe = i;
            break;
        }
    }
    // 不存在pipe
    if (positionOfPipe == -1)
        executeWithoutPipe(start, end, argv);
    // 存在pipe
    // write to pipe[1]
    // read from pipe[0]
    int pipe1[2];
    if (pipe(pipe1) < 0)
        exit(EXIT_FAILURE);
    pid_t pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    else if (pid == 0) {
        // 子进程
        close(pipe1[0]);
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);
        executeWithoutPipe(start, positionOfPipe, argv);
    } else if (pid > 0) {
        // 父进程等待子进程执行完毕
        int status = 0;
        waitpid(pid, &status, 0);
        if (positionOfPipe + 1 < end) {
            close(pipe1[1]);
            dup2(pipe1[0], STDIN_FILENO);
            close(pipe1[0]);
            executeWithPipe(positionOfPipe + 1, end, argv);
        }
    }
}

void executeWithoutPipe(int start, int end, char **argv) {
    int numberOfInputs = 0, numberOfOutputs = 0;
    char *fileOfInputs = NULL, *fileOfOutputs = NULL;
    int endOfRedirection = end;
    // 寻找重定向符号
    for (int i = start; i < end; i++) {
        if (strcmp(argv[i], "<") == 0) {
            // 输入重定向
            numberOfInputs++;
            fileOfInputs = argv[i + 1];
            if (endOfRedirection == end)
                endOfRedirection = i;
        } else if (strcmp(argv[i], ">") == 0) {
            // 输出重定向
            numberOfOutputs++;
            fileOfOutputs = argv[i + 1];
            if (endOfRedirection == end)
                endOfRedirection = i;
        }
    }

    if (numberOfInputs == 1) {
        // close stdin
        close(0);
        // open file as stdin
        open(fileOfInputs, O_RDONLY);
    }
    if (numberOfOutputs == 1) {
        // close stdout
        close(1);
        // open file as stdout
        open(fileOfOutputs, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    char *temp[PARAMETER_SIZE];
    for (int i = start; i < end; i++)
        temp[i] = argv[i];
    temp[endOfRedirection] = NULL;
    execvp(temp[start], temp + start);
}