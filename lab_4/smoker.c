#include "ipc.h"

int main (int argc, char *argv[]) {
    int rate;
    // 可在在命令行第一参数指定一个进程睡眠秒数，以调解进程执行速度
    if (argv[2] != NULL)
        rate = atoi(argv[2]);
    else
        // 默认指定为 3 秒
        rate = 3;
    // 共享内存使用的变量
    buff_key = 101;     // 缓冲区任给的键值
    buff_num = 1;       // 缓冲区任给的长度
    shm_flg = IPC_CREAT | 0644;     // 共享内存读写权限 
    // 获取缓冲区使用的共享内存，seq_ptr 指向缓冲区首地址
    seq_ptr = (int *)set_shm(buff_key, buff_num, shm_flg); 
    // 信号量使用的变量
    sem_flg = IPC_CREAT | 0644;
    // 获取吸烟者同步信号量
    smoker_key_1 = 301;
    smoker_key_2 = 302;
    smoker_key_3 = 303;
    smoker_sem_1 = set_sem(smoker_key_1, sem_val, sem_flg);
    smoker_sem_2 = set_sem(smoker_key_2, sem_val, sem_flg);
    smoker_sem_3 = set_sem(smoker_key_3, sem_val, sem_flg);
    sem_val = 0;
    int smoker_sem = 0;
    switch (atoi(argv[1])) {
        case 1:
            smoker_sem = smoker_sem_1;
            break;
        case 2:
            smoker_sem = smoker_sem_2;
            break;
        case 3:
            smoker_sem = smoker_sem_3;
            break;
    }
    // 供应者互斥sem_val = 1
    sem_val = 1;
    supplier_key = 201;
    supmutex_key = 202;
    // 获取供应者同步信号量
    supplier_sem = set_sem(supplier_key, sem_val, sem_flg);
    // 获取供应者互斥信号量
    supmutex_sem = set_sem(supmutex_key, sem_val, sem_flg);
    // 循环执行模拟供应者不断放产品
    while(1) {
        // 互斥
        // 一开始可以供应材料
        P(smoker_sem);

        if (*seq_ptr == 0) {
            printf("smoker: 收到供应烟草、纸\n");
        } else if (*seq_ptr == 1) {
            printf("smoker: 收到供应烟草、胶水\n");
        } else if (*seq_ptr == 2) {
            printf("smoker: 收到供应纸、胶水\n");
        }
        sleep(rate);
        // 解除互斥
        V(supplier_sem);
    }

    return EXIT_SUCCESS; 
}