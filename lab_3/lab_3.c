#include "lab_3.h"
char *history[HISTORY_SIZE];
int background = 0;
int firstOfHistory, historyCount = 0;

int main() {
    // need to free
    char *argv[PARAMETER_SIZE];
    char *inputLine = (char *)malloc(EXE_LENGTH);
    char *temp = (char *)malloc(EXE_LENGTH);
    char *currentDir = (char *)malloc(MAX_PATH);
    size_t length = 0;
    
    for (int i = 0; i < PARAMETER_SIZE; i++)
        argv[i] = (char *)malloc(EXE_LENGTH);
    
    for (int i = 0; i < HISTORY_SIZE; i++)
        history[i] = (char *)malloc(EXE_LENGTH);

    while (1) {
        // 获取当前目录
        getcwd(currentDir, MAX_PATH);
        printf("Shell:~%s$ ", currentDir);
        // SIGNAL
        signal(SIGINT,(sighandler_t)sigcat);
        // 处理输入
        fgets(inputLine, EXE_LENGTH, stdin);
        length = strlen(inputLine);
        // Enter
        if (strcmp(inputLine, "\n") == 0)
            continue;
        inputLine[length - 1] = '\0';
        // 后台运行
        if (inputLine[length - 2] == '&')
            background = 1;

        // need to free
        strcpy(temp, inputLine);
        size_t argc = parse(temp, argv);
        if (argc > 0)
            execute(argc, argv);
        // 存储历史记录
        strcpy(history[(firstOfHistory + 1) % HISTORY_SIZE], inputLine);
        firstOfHistory = (firstOfHistory + 1) % HISTORY_SIZE;
        historyCount++;
    }

    if (inputLine != NULL) {
        free(inputLine);
        inputLine = NULL;
    }
    if (temp != NULL) {
        free(temp);
        temp = NULL;
    }
    if (currentDir != NULL) {
        free(currentDir);
        currentDir = NULL;
    }
    for (int i = 0; i < PARAMETER_SIZE; i++) {
        free(argv[i]);
        argv[i] = NULL;
    }
    for (int i = 0; i < HISTORY_SIZE; i++) {
        free(history[i]);
        history[i] = NULL;
    }

    return 0;
}