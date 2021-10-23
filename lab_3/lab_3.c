#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#define hist_size 1024 
char *hist[hist_size];
int f = 0;   //to save change in directory
int head = 0, filled = 0;
 
 
//1-parse user's input
void parse(char *word, char **argv)
{
    int count = 0;
    memset(argv, 0, sizeof(char*)*(64));//copies 0 to the first 64 characters of the string pointed to by the argument argv.
    char *lefts = NULL;
    const char *split = " ";  //setting delimeter
    while (1)
    {
        char *p = strtok_r(word, split, &lefts);//ref-"strtok-r":http://baike.baidu.com/view/6991509.htm?fr=aladdin
        if (p == NULL)
        {
            break;
        }
        argv[count] = p;//argv is an array, it stores each value of your input divided by " "
        word = lefts;
        //printf("%s\n",argv[count]);
        //printf("%s\n",word);
        count++;
    }
    if (strcmp(argv[0], "exit") == 0)
        exit(0);
    else if (strcmp(argv[0], "cd") == 0)
    {
        int ch = chdir(argv[1]); //ref-"chdir":http://baike.baidu.com/view/653970.htm?fr=aladdin
        /*if(ch<0)
        {
        printf("No such file or directory %d\n.",ch);
        }*/
        f = 1;
    }
}
 
 
//2-get the first word
char *trim(char *string)//remove extra spaces
{
    int i = 0;
    int j = 0;
    char *ptr = malloc(sizeof(char*)*strlen(string));
    for (i = 0; string[i] != '\0'; i++)
    if (string[i] != ' ')
    {
        ptr[j] = string[i];
        j++;
    }
    ptr[j] = '\0';
    string = ptr;
    //printf("%s\n",string);
    //printf("%d\n",j);
    return string;
}
 
 
//3-execute the basic order
void execute(char **argv)
{
    pid_t pid;
    int status;
    //fork child process
    if ((pid = fork()) < 0)
    {
        printf("error:fork failed.\n");
        exit(1);
    }
    else if (pid == 0)
    {
        if (execvp(argv[0], argv) < 0 && strcmp(argv[0], "cd"))//ref-"execvp":http://baike.baidu.com/view/1745420.htm?fr=aladdin
            printf("error:invalid command.\n");
        exit(0);
    }
    else
    {
        while (wait(&status) != pid)//ref-"wait":http://see.xidian.edu.cn/cpp/html/289.html
            ;
    }
}
 
 
//4-output redirect
//output:file in which we need to print the output
void  execute_file(char **argv, char *output)
{
    //printf("output:%s\n",output);
    //printf("argv:%s\n",*argv);
    pid_t pid;
    int status, flag;
    char *file = NULL;
    if ((pid = fork()) < 0)
    {
        printf("error:fork failed.\n");
        exit(1);//fclose(fd1);
    }
    else if (pid == 0)
    {
        if (strstr(output, ">")>0)//returns a pointer to first occurence after > or null
        {
            char *p = strtok_r(output, ">", &file);
            output += 1;   //change2
            file = trim(file);//get the first word of file, that is to say, get the first word after >
            flag = 1;
            //printf("file : %s output : %s \n",*argv,output);
            int old_stdout = dup(1);
            //printf("mark\n");
            //output redirect
            FILE *fp1 = freopen(output, "w+", stdout);//ref-"freopen":http://baike.baidu.com/view/656692.htm?fr=aladdin
            //printf("mark\n");
            execute_file(argv, file);
            fclose(stdout);
            FILE *fp2 = fdopen(old_stdout, "w");//ref-"fdopen":http://baike.baidu.com/view/656646.htm?fr=aladdin
            *stdout = *fp2;
            exit(0);
        }
        if (strstr(output, "<") > 0)
        {
            char *p = strtok_r(output, "<", &file);
            file = trim(file);
            flag = 1;
            int fd = open(file, O_RDONLY);//ref-"open":http://see.xidian.edu.cn/cpp/html/238.html
            if (fd<0)
            {
                printf("No such file or directory.");
                exit(0);
            }
        }
        if (strstr(output, "|")>0)
        {
            fflush(stdout); printf("here"); fflush(stdout);
            char *p = strtok_r(output, "|", &file);
            file = trim(file);
            flag = 1;
            //fflush(stdout);printf("%s",file);fflush(stdout);
            char *args[64];
            parse(file, args);
            execute(args);
        }
        int old_stdout = dup(1);
        FILE *fp1 = freopen(output, "w+", stdout);
        if (execvp(argv[0], argv) < 0)
            printf("error:in exec");
        fclose(stdout);
        FILE *fp2 = fdopen(old_stdout, "w");
        *stdout = *fp2;
        exit(0);
    }
    else
    {
        while (wait(&status) != pid)
            ;
    }
}
 
 
//5-input redirect
void  execute_input(char **argv, char *output)
{
    pid_t pid;
    int fd;
    char *file;
    int flag = 0;
    int status;
    if ((pid = fork()) < 0)
    {
        printf("error:fork failed\n");
        exit(1);
    }
    else if (pid == 0)
    {
        if (strstr(output, "<")>0)
        {
            char *p = strtok_r(output, "<", &file);
            file = trim(file);
            flag = 1;
            //printf("file : %s output : %s \n",file,output);
            fd = open(output, O_RDONLY);
            if (fd<0)
            {
                printf("No such file or directory.");
                exit(0);
            }
            output = file;
        }
        if (strstr(output, ">")>0)
        {
            char *p = strtok_r(output, ">", &file);
            file = trim(file);
            flag = 1;
            fflush(stdout);
            //printf("file : %s output : %s \n",file,output);
            fflush(stdout);
            int old_stdout = dup(1);
            FILE *fp1 = freopen(file, "w+", stdout);
            execute_input(argv, output);
            fclose(stdout);
            FILE *fp2 = fdopen(old_stdout, "w");
            *stdout = *fp2;
            exit(0);
        }
        if (strstr(output, "|") > 0)
        {
            char *p = strtok_r(output, "|", &file);
            file = trim(file);
            flag = 1;
            char *args[64];
            parse(file, args);
            int pfds[2];
            pid_t pid, pid2;
            int status, status2;
            pipe(pfds);
            int fl = 0;
            if ((pid = fork()) < 0)
            {
                printf("error:fork failed\n");
                exit(1);
            }
            if ((pid2 = fork()) < 0)
            {
                printf("error:fork failed\n");
                exit(1);
            }
            if (pid == 0 && pid2 != 0)
            {
                close(1);
                dup(pfds[1]);
                close(pfds[0]);
                close(pfds[1]);
                fd = open(output, O_RDONLY);
                close(0);
                dup(fd);
                if (execvp(argv[0], argv) < 0)
                {
                    close(pfds[0]);
                    close(pfds[1]);
                    printf("error:in exec");
                    fl = 1;
                    exit(0);
                }
                close(fd);
                exit(0);
            }
            else if (pid2 == 0 && pid != 0 && fl != 1)
            {
                close(0);
                dup(pfds[0]);
                close(pfds[1]);
                close(pfds[0]);
                if (execvp(args[0], args) < 0)
                {
                    close(pfds[0]);
                    close(pfds[1]);
                    printf("error:in exec");
                    exit(0);
                }
            }
            else
            {
                close(pfds[0]);
                close(pfds[1]);
                while (wait(&status) != pid);
                while (wait(&status2) != pid2);
            }
            exit(0);
        }
        fd = open(output, O_RDONLY);
        close(0);
        dup(fd);
        if (execvp(argv[0], argv) < 0)
        {
            printf("error:in exec");
        }
        close(fd);
        exit(0);
    }
    else
    {
        while (wait(&status) != pid);
    }
 
}
 
 
 
//6-implement pipe
void execute_pipe(char **argv, char *output)
{
    int pfds[2], pf[2], flag;
    char *file;
    pid_t pid, pid2, pid3;
    int status, status2, old_stdout;
    pipe(pfds);//create pipe
    //pfds[0]:read        pfds[1]:write
    int blah = 0;
    char *args[64];
    char *argp[64];
    int fl = 0;
    if ((pid = fork()) < 0)
    {
        printf("error:fork failed\n");
        exit(1);
    }
    if ((pid2 = fork()) < 0)
    {
        printf("error:fork failed\n");
        exit(1);
    }
    if (pid == 0 && pid2 != 0)
    {
        close(1);
        dup(pfds[1]);
        close(pfds[0]);
        close(pfds[1]);
        if (execvp(argv[0], argv) < 0)//run the command
        {
            close(pfds[0]);
            close(pfds[1]);
            printf("error:in exec");
            fl = 1;
            kill(pid2, SIGUSR1);
            exit(0);
        }
    }
    else if (pid2 == 0 && pid != 0)
    {
        if (fl == 1){ exit(0); }
        if (strstr(output, "<") > 0)
        {
            char *p = strtok_r(output, "<", &file);
            file = trim(file);
            flag = 1;
            parse(output, args);//divide output to the array args
            execute_input(args, file);
            close(pfds[0]);
            close(pfds[1]);
            exit(0);
        }
        if (strstr(output, ">") > 0)
        {
            char *p = strtok_r(output, ">", &file);
            file = trim(file);
            flag = 1;
            //fflush(stdout);printf("file : %s output : %s \n",file,output);fflush(stdout);
            parse(output, args);
            blah = 1;
        }
 
        else
        {
            parse(output, args);
        }
        close(0);
        dup(pfds[0]);
        close(pfds[1]);
        close(pfds[0]);
        if (blah == 1)
        {
            old_stdout = dup(1);
            FILE *fp1 = freopen(file, "w+", stdout);
        }
        if (execvp(args[0], args) < 0)
        {
            fflush(stdout);
            printf("error:in exec %d", pid);
            kill(pid, SIGUSR1);
            close(pfds[0]);
            close(pfds[1]);
        }
        fflush(stdout);
        printf("HERE");
        //kill (pid, SIGUSR1);
        if (blah == 1)
        {
            fclose(stdout);
            FILE *fp2 = fdopen(old_stdout, "w");
            *stdout = *fp2;
        }
    }
    else
    {
        close(pfds[0]);
        close(pfds[1]);
        while (wait(&status) != pid);
        while (wait(&status2) != pid2);
    }
}
 
 
 
//7-implement pipe
void execute_pipe2(char **argv, char **args, char **argp)
{
    int status;
    int i;
    int pipes[4];
    pipe(pipes);
    pipe(pipes + 2);
    if (fork() == 0)
    {
        dup2(pipes[1], 1);
        close(pipes[0]);
        close(pipes[1]);
        close(pipes[2]);
        close(pipes[3]);
        if (execvp(argv[0], argv) < 0)
        {
            fflush(stdout);
            printf("error:in exec");
            fflush(stdout);
            close(pipes[0]);
            close(pipes[1]);
            close(pipes[2]);
            close(pipes[3]);
            exit(1);
        }
    }
    else
    {
        if (fork() == 0)
        {
            dup2(pipes[0], 0);
            dup2(pipes[3], 1);
            close(pipes[0]);
            close(pipes[1]);
            close(pipes[2]);
            close(pipes[3]);
            if (execvp(args[0], args) < 0)
            {
                fflush(stdout);
                printf("error:in exec");
                fflush(stdout);
                close(pipes[0]);
                close(pipes[1]);
                close(pipes[2]);
                close(pipes[3]);
                exit(1);
            }
        }
        else
        {
            if (fork() == 0)
            {
                dup2(pipes[2], 0);
                close(pipes[0]);
                close(pipes[1]);
                close(pipes[2]);
                close(pipes[3]);
                if (execvp(argp[0], argp) < 0)
                {
                    fflush(stdout);
                    printf("error:in exec");
                    fflush(stdout);
                    close(pipes[0]);
                    close(pipes[1]);
                    close(pipes[2]);
                    close(pipes[3]);
                    exit(1);
                }
            }
        }
    }
    close(pipes[0]);
    close(pipes[1]);
    close(pipes[2]);
    close(pipes[3]);
    for (i = 0; i < 3; i++)
        wait(&status);
}
 
/*
int main()
{
char word[1024]="ls>a.txt";
char *argv[1024];
char *file=NULL;
char *p=strtok_r(word ,">", &file);
file=trim(file);
parse(word,argv);
execute_file(argv,file);
}*/
 
 
 
int  main()
{
    char line[1024];
    char *argv[64];
    char *args[64];
    char *left;
    size_t size = 0;
    char ch;
    int count = 0;
    char *tri;
    char *second;
    char *file;
    int i;
    for (i = 0; i < hist_size; i++)
    {
        hist[i] = (char *)malloc(150);
    }
 
    while (1)
    {
        count = 0;
        int flag = 0;
        char *word = NULL;
        char *dire[] = { "pwd" };
        fflush(stdout);
        printf("SHELL~");
        fflush(stdout);
        execute(dire); //print the current directory, we can also use getcwd()
        printf("$");
        int len = getline(&word, &size, stdin);
        if (*word == '\n')
            continue;
        word[len - 1] = '\0';
        char *file = NULL;
        int i = 0;
        char *temp = (char *)malloc(150);
        strcpy(temp, word);
        parse(temp, argv);
 
        strcpy(hist[(head + 1) % hist_size], word);   //storing an entry in history
        head = (head + 1) % hist_size;
        filled = filled + 1;
 
        for (i = 0; word[i] != '\0'; i++)
        {
 
            if (word[i] == '>')
            {
                //printf("%s\n",word);   //has the initial command
                char *p = strtok_r(word, ">", &file);
                file = trim(file);
                //printf("%s\n",file); 
                flag = 1;
                break;
            }
            else if (word[i] == '<')
            {
                char *p = strtok_r(word, "<", &file);
                file = trim(file);
                //printf("file : %s",file);
                flag = 2;
                break;
            }
            else if (word[i] == '|')
            {
                char *p = strtok_r(word, "|", &left);
                flag = 3;
                break;
            }
        }
        if (strcmp(word, "exit") == 0)
        {
            exit(0);
        }
 
        if (flag == 1)
        {
            parse(word, argv);   //parsed command stored in argv
            //printf("At the right place");
            execute_file(argv, file);
        }
        else if (flag == 2)
        {
            parse(word, argv);
            execute_input(argv, file);
        }
        else if (flag == 3)
        {
            char *argp[64];
            char *output, *file;
            if (strstr(left, "|") > 0)
            {
                char *p = strtok_r(left, "|", &file);
                parse(word, argv);
                parse(left, args);
                parse(file, argp);
                execute_pipe2(argv, args, argp);
            }
            else
            {
                parse(word, argv);
                execute_pipe(argv, left);
            }
        }
        else
        {
            parse(word, argv);
            execute(argv);
        }
    }
 
}