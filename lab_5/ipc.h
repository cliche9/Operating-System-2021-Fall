/*
* Filename
* copyright
* Function
: ipc.h
: (C) 2006 by zhonghonglie
: 声明 IPC 机制的函数原型和全局变量
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#define BUFFERSIZE 256
#define MAXVAL 100
#define STRSIZE 8
#define WRITERQUEST 1       // 写标志
#define READERQUEST 2       // 读标志
#define FINISHED 3          // 完成标志

/* 信号量控制用的共同体 */
typedef union semuns {
    int val;
} Sem_uns;

/* 消息结构体 */
typedef struct msgbuf {
    long mtype;
    int mid;
    int tid;
} Msg_buf;

key_t buff_key;
int buff_num;
char *buff_ptr;
int shm_flg;

int quest_flg;
key_t quest_key;
int quest_id;

int respond_flg;
key_t respond_key;
int respond_id;

// room请求sofa相关变量
int sofa_quest_flg;
key_t sofa_quest_key;
int sofa_quest_id;
// sofa响应room相关变量
int sofa_respond_flg;
key_t sofa_respond_key;
int sofa_respond_id;
// sofa请求barber相关变量
int barber_quest_flg;
key_t barber_quest_key;
int barber_quest_id;
// barber响应sofa相关变量
int barber_respond_flg;
key_t barber_respond_key;
int barber_respond_id;
// customer请求account相关变量
int account_quest_flg;
key_t account_quest_key;
int account_quest_id;
// barber响应account相关变量
int account_respond_flg;
key_t account_respond_key;
int account_respond_id;
// 顾客相关信号量
key_t customer_key;
int customer_sem;
// 理发座椅相关信号量
key_t chair_key;
int chair_sem;
// 账本相关信号量
key_t account_key;
int account_sem;
// 信号量赋值/flag
int sem_flg;
int sem_val;

int get_ipc_id(char *proc_file, key_t key);
char *set_shm(key_t shm_key, int shm_num, int shm_flag);
int set_msq(key_t msq_key, int msq_flag);
int set_sem(key_t sem_key, int sem_val, int sem_flag);
int sem_wait(int sem_id);
int sem_signal(int sem_id);