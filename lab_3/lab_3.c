#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>
// 指令长度 历史记录 路径长度 参数个数
#define EXE_LENGTH 200
#define USER_LENGTH 64
#define PARAMETER_SIZE 64
#define HISTORY_SIZE 1024
#define MAX_PATH 1024
typedef void(*sighandler_t)(int);

// char history[HISTORY_SIZE][EXE_LENGTH];
char currentDir[MAX_PATH];
char userPath[USER_LENGTH] = "myshell@TEST ";
char prompt[MAX_PATH];
char *inputLine = NULL;
int background = 0;
// int firstOfHistory = 0, historyCount = 0;

void sigcat(int sigNumber) {
    printf("\nCatch Signal %d\n", sigNumber);
}

void sigcat_quit(int sigNumber) {
    exit(EXIT_FAILURE);
}

static char *processInput();
// 1-parse user's input
static int parse(char *word, char *argv[]);
//2-get the first word
static void trim(char *string);
//3-execute order
static void executeWithoutPipe(int start, int end, char **argv);
static void executeWithPipe(int start, int end, char **argv);
static void execute(int argc, char *argv[], int background);

int main() {
    char argv[PARAMETER_SIZE][EXE_LENGTH];
    char *inputLine;
    char temp[EXE_LENGTH];
    size_t length = 0;

    while (1) {
        // SIGNAL
        signal(SIGINT,(sighandler_t)sigcat);
        // 处理输入
        inputLine = processInput();
        length = strlen(inputLine);
        // Enter
        if (strcmp(inputLine, "\n") == 0)
            continue;
        // History
        // 后台运行
        if (inputLine[length - 1] == '&') {
            background = 1;
            inputLine[length - 1] = '\0';
        }
        // printf("EXE: %s\n", inputLine);

        strcpy(temp, inputLine);
        size_t argc = parse(temp, argv);
        if (argc > 0)
            execute(argc, argv, background);
        // 存储历史记录
        // strcpy(history[firstOfHistory], inputLine);
        // firstOfHistory = (firstOfHistory + 1) % HISTORY_SIZE;
        // historyCount++;
    }
    
    return 0;
}

char *processInput() {
    // 获取当前目录
    strcpy(prompt, userPath);
    background = 0;
    getcwd(currentDir, MAX_PATH);
    strcat(prompt, currentDir);
    strcat(prompt, " $ ");
    
    if (inputLine) {
        free(inputLine);
        inputLine = NULL;
    }
    
    inputLine = readline(prompt);
    // printf("\ninputLine: %s\n", inputLine);

    if (inputLine && *inputLine)
        add_history(inputLine);

    return inputLine;
}

int parse(char *word, char *argv[]) {
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

void execute(int argc, char *argv[], int background) {
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
        // 这里不会运行 是为啥呢?
        if (background == 1) {
            printf("\n[PID] %d + done\t", getpid());
            for (int i = 0; i < argc; i++)
                printf("%s ", argv[i]);
            printf("\n");
        }
    } else {
        if (background == 0)
            while (wait(&status) != pid); //ref-"wait":http://see.xidian.edu.cn/cpp/html/289.html
        else {
            printf("[PID] %d\n", pid);         
        }
    }
}

void executeWithPipe(int start, int end, char *argv[]) {
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
    if (positionOfPipe == -1) {
        executeWithoutPipe(start, end, argv);
        return;
    }
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
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);
        close(pipe1[0]);
        executeWithoutPipe(start, positionOfPipe, argv);
        exit(EXIT_SUCCESS);
    } else if (pid > 0) {
        // 父进程等待子进程执行完毕
        int status = 0;
        waitpid(pid, &status, 0);
        if (positionOfPipe + 1 < end) {
            dup2(pipe1[0], STDIN_FILENO);
            // 为什么要完全关闭管道？
            close(pipe1[0]);
            close(pipe1[1]);
            executeWithPipe(positionOfPipe + 1, end, argv);
        }
    }
}

// find . -maxdepth 1 -name 111.txt | xargs grep hello | wc

void executeWithoutPipe(int start, int end, char *argv[]) {
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
    // execvp(temp[start], temp + start);
    // 调用子程序执行execvp, 方便输出信息
    pid_t pid = fork();
    if (pid < 0) {
        perror("Create Process Failed.\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0)
        execvp(temp[start], temp + start);
    else {
        int status = 0;
        waitpid(pid, &status, 0);
    }
}