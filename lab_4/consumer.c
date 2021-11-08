/* 
Filename: consumer.c
copyright: (C) by zhanghonglie
Function: 建立并模拟消费者进程
*/

#include "ipc.h"

int main(int argc, char *argv[]) {
    int rate;
    if (argv[1] != NULL)
        rate = atoi(argv[1]);
    else 
        rate = 3;
    // sharing memory
    buff_key = 101;     // 缓冲区键值
    buff_num = 8;       // 缓冲区长度
    cget_key = 103;     // 消费者取产品指针键值
    cget_num = 1;       // 指针数目
    // 共享内存读写权限
    shm_flg = IPC_CREAT | 0644;
    // 获取缓冲区使用的共享内存, buff_ptr 指向缓冲区首地址
    buff_ptr = (char *)set_shm(buff_key, buff_num, shm_flg);
    // 获取消费者取产品指针, cget_ptr 指向索引地址
    cget_ptr = (int *)set_shm(cget_key, cget_num, shm_flg);
    // 信号量使用的变量
    prod_key = 201;     // 生产者同步信号量
    pmtx_key = 202;     // 生产者互斥信号量
    cons_key = 301;     // 消费者同步信号量
    cmtx_key = 302;     // 消费者同步信号量
    sem_flg = IPC_CREAT | 0644;     // 信号量操作权限
    // get/set生产者同步信号量初值为缓冲区最大值
    sem_val = buff_num;
    prod_sem = set_sem(prod_key, sem_val, sem_flg);
    // get/set消费者同步信号量 = 0
    sem_val = 0;
    cons_sem = set_sem(cons_key, sem_val, sem_flg);
    // get/set消费者互斥信号量 = 1
    sem_val = 1;
    cmtx_sem = set_sem(cmtx_key, sem_val, sem_flg);
    
    // 循环, 消费者取产品
    while (1) {
        // 无产品, 消费者阻塞
        down(cons_sem);
        // 另一个消费者取产品, 互斥，本消费者阻塞
        down(cmtx_sem);
        
        // 读1个char作为模拟区产品, 报告进程号、字符char、读取位置
        sleep(rate);
        printf("%d consumer get: %c from Buffer[%d]\n", getpid(), buff_ptr[*cget_ptr], *cget_ptr);
        // 读取位置循环下移
        *cget_ptr = (*cget_ptr + 1) % buff_num;

        // 唤醒互斥阻塞的消费者
        up(cmtx_sem);
        // 唤醒阻塞的生产者
        up(prod_sem);
    }

    return EXIT_SUCCESS;
}