/* 
 * Filename: customer.c
 * Function: 建立并模拟顾客进程
 */

#include "ipc.h"

int main(int argc, char *argv[]) {
    int rate;
    if(argv[1] != NULL)
        rate = atoi(argv[1]);
    else 
        rate = 1;   
     
    Msg_buf msg_arg;
    // 建立room队列, 可容纳13人
    sofa_quest_flg = IPC_CREAT | 0644;
    sofa_quest_key = 111;
    sofa_quest_id = set_msq(sofa_quest_key, sofa_quest_flg);
    // 建立barber队列, 可容纳4人
    barber_quest_flg = IPC_CREAT | 0644;
    barber_quest_key = 211;    
    barber_quest_id = set_msq(barber_quest_key, barber_quest_flg);    
    barber_respond_flg = IPC_CREAT | 0644;
    barber_respond_key = 212;    
    barber_respond_id = set_msq(barber_respond_key, barber_respond_flg);
    // 建立account请求队列, 可容纳3个
    account_quest_flg = IPC_CREAT | 0644;
    account_quest_key = 213;
    account_quest_id = set_msq(account_quest_key, account_quest_flg);
    account_respond_flg = IPC_CREAT | 0644;
    account_respond_key = 214;    
    account_respond_id = set_msq(account_respond_key, account_respond_flg);  
    // 建立账本信号量
    account_key = 312;    
    sem_flg = IPC_CREAT | 0644;
    sem_val = 1;    
    account_sem = set_sem(account_key, sem_val, sem_flg);
    // 共享内存
    buff_key = 101;
    buff_num = STRSIZE + 1;
    shm_flg = IPC_CREAT | 0644;
    buff_ptr = (char *)set_shm(buff_key, buff_num, shm_flg);
    // 沙发上剩余座位
    buff_ptr[SOFA] = 4;   
    // 等待室剩余座位
    buff_ptr[ROOM] = 13;
    // 椅子剩余座位
    buff_ptr[CHAIR] = 3;
    // 顾客id 
    int id = -1;

    while (1) {
        sleep(rate);
        id++;
        msg_arg.mid = id;
        msg_arg.mtype = id + 1;
        // 来了新customer, 先让新顾客坐下, 然后处理请求
        if (buff_ptr[SOFA] > 0) {
            // 沙发有空位
            if (buff_ptr[ROOM] == 13) {
                // 等待室没人
                printf("新顾客 %d 直接坐在沙发上\n", id);
                buff_ptr[SOFA]--;
                // 请求理发
                msg_arg.mtype = HAIRQUEST;
                msgsnd(barber_quest_id, &msg_arg, sizeof(msg_arg), 0);
            } else {
                // 等待室有人, 新顾客先进入等待室, 然后再请求
                printf("新顾客 %d 进入等待室等待进入沙发\n", id);
                buff_ptr[ROOM]--;
                msg_arg.mtype = SOFAQUEST;
                msgsnd(sofa_quest_id, &msg_arg, sizeof(msg_arg), 0);
            }
        } else if (buff_ptr[ROOM] > 0){
            // 沙发满了, 等待室有空位
            printf("沙发满, 新顾客 %d 只能进入等待室等待\n", id);
            buff_ptr[ROOM]--;
            // 请求进入沙发
            msg_arg.mtype = SOFAQUEST;
            msgsnd(sofa_quest_id, &msg_arg, sizeof(msg_arg), 0);
        } else
            // 等待室和沙发都满了, customer离开
            printf("理发店满员, 顾客 %d 离开\n", id);

        // 处理沙发请求
        if (msgrcv(sofa_quest_id, &msg_arg, sizeof(msg_arg), SOFAQUEST, IPC_NOWAIT) >= 0) {
            buff_ptr[SOFA]--;
            buff_ptr[ROOM]++;
            printf("顾客 %d 从等待室坐到沙发上\n", msg_arg.mid);
            if (buff_ptr[CHAIR] > 0) {
                msg_arg.mtype = HAIRQUEST;
                msgsnd(barber_quest_id, &msg_arg, sizeof(msg_arg), 0);
                printf("顾客 %d 请求理发\n", msg_arg.mid);
            }
        }
        // 接收理发结束回应
        if (msgrcv(barber_respond_id, &msg_arg, sizeof(msg_arg), CUT_FINISHED, IPC_NOWAIT) >= 0) {
            printf("顾客 %d 理发完毕\n", msg_arg.mid);
            msg_arg.mtype = ACCOUNTQUEST;
            printf("顾客 %d 请求结账\n", msg_arg.mid);
            msgsnd(account_quest_id, &msg_arg, sizeof(msg_arg), 0);
        }
        // 接收结账结束回应
        if (msgrcv(account_respond_id, &msg_arg, sizeof(msg_arg), ACC_FINISHED, IPC_NOWAIT) >= 0) {
            printf("顾客 %d 结账完毕\n", msg_arg.mid);
            buff_ptr[CHAIR]++;
            printf("顾客 %d 离开理发店\n", msg_arg.mid);
        }
    }

    return EXIT_SUCCESS;
}
