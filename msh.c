/*
    Myshell
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
int pid;
typedef void(*sighandler_t)(int);
void sigcat(){//信号处理
    kill(pid,SIGINT);

}
char *arg[100];
void get_input(char *);          //获得用户的输入
int explain_input(char *);
void do_command(int);
void do_command_nopipe(int s,int e);
void do_command_pipe(int s,int e);
// redirect();//恢复重定向
int main(int argc,char **argv)
{
    int i;
    for(i=0;i<100;i++)
        arg[i]=(char *)malloc(256);      
    
    char *bufe=NULL;   //将用户输入的命令存储到缓冲区中
    bufe=(char *)malloc(256);
    while(!feof(stdin)){
        int argcount=0;
        if(bufe==NULL)
        {
            perror("apply for malloc failed!");
            exit(-1);
        }
        memset(bufe,0,256);
        signal(SIGINT,(sighandler_t)sigcat);
        get_input(bufe);//进行输入
        int len=strlen(bufe);
        bufe[len-1]='\0'; 
        argcount=explain_input(bufe);
        if(argcount!=0)
            do_command(argcount);
        
    }
    if(bufe!=NULL)
    {
        free(bufe);
        bufe=NULL;
    }
    return 0;
}
void do_command(int argcount){
    pid=fork();
    if(pid==-1){
        exit(EXIT_FAILURE);
    }
    else if(pid==0){
        int ifd=dup(STDIN_FILENO);
        int ofd=dup(STDOUT_FILENO);
        do_command_pipe(0,argcount);
        dup2(ifd,STDIN_FILENO);
        dup2(ofd,STDOUT_FILENO);
    }
    else if(pid>0){
        int status;
        waitpid(pid,&status,0);
        return;
    }
}

void get_input(char *bufe)
{
    fgets(bufe,256,stdin);
}   
int explain_input(char* bufe)
{
    char *p=bufe;
    char *q=bufe;
    int num=0;
    int argcou=0;
    while(1)
    {
        if(p[0]=='\0')
            break;
        if(p[0]==' ')
            p++;
        else
        {
            q=p;
            num=0;
            while((q[0]!=' ')&&(q[0]!='\0'))
            {
                num++;
                q++;
            }
            strncpy(arg[argcou],p,num);
            arg[argcou][num] = '\0';
            argcou++;
            p=q;
        }
        
    }
    return argcou;
}
void do_command_pipe(int s,int e)
{
    if(s>=e)
        return;
    int p_position=-1;
    for(int i=s;i<e;i++){
        if(strcmp(arg[i],"|")==0){
            p_position=i;//记录管道位置
            break;
        }
    }
    if(p_position==-1){
        do_command_nopipe(s,e);
       
    }
    //printf("测试点1\n");
    int pipe1[2];
    if(pipe(pipe1)==-1){
        exit(EXIT_FAILURE);
    }
    int pid =fork();
    if(pid==-1)
    {
        exit(EXIT_FAILURE);
    }
    else if(pid==0){
        close(pipe1[0]);
        dup2(pipe1[1],STDOUT_FILENO);
        close(pipe1[1]);
        do_command_nopipe(s,p_position);
    }
    else if(pid>0)
    {
        int status;
        waitpid(pid,&status,0);//等待子进程执行完毕
        if(p_position+1<e){
            close(pipe1[1]);
            dup2(pipe1[0],STDIN_FILENO);
            close(pipe1[0]);
            do_command_pipe(p_position+1,e);
        }
    }
}
void do_command_nopipe(int s,int e)
{
    int in_num=0,out_num=0;
    char *in_file=NULL,*out_file=NULL;
    int end=e;
    //查询重定向符是否存在
    for(int i=s;i<e;i++){
        if(strcmp(arg[i],"<")==0){
            in_num++;
            in_file=arg[i+1];
            if(end==e)
                end=i;
        }
        if(strcmp(arg[i],">")==0){
            out_num++;
            out_file=arg[i+1];
            if(end==e)
                end=i;
        }
    }
   // printf("测试\n");
    if(in_num==1){
        close(0);
        open(in_file,O_RDONLY);
    }
    if(out_num==1)
    {
        close(1);
        open(out_file,O_WRONLY|O_CREAT|O_TRUNC,0644);
    }
    char *temp[256];
    for(int i=s;i<end;i++)
        temp[i]=arg[i];
    temp[end]=NULL;
    execvp(temp[s],temp+s);

}


