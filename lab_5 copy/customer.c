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
        // 来了新customer
        if (buff_ptr[SOFA] < 4) {
            // 沙发有空位
            if (wait_count > 0) {
                // 等待室有人
                sofa_quest_flg = IPC_NOWAIT;
                // sofa收到waiting room请求进入sofa, 请求是非阻塞的, 这个请求一定是room中时间最长的customer的quest
                msgrcv(sofa_quest_id, &msg_arg, sizeof(msg_arg), 0, sofa_quest_flg);
                // sofa有空, 允许customer从waiting room进入sofa, respond是阻塞的, 同时只能回应一个customer
                msgsnd(sofa_respond_id, &msg_arg, sizeof(msg_arg), 0);
            } else {
                // 等待室没人, customer直接进入sofa
                printf("a new customer %d sit at sofa\n", id);
                buff_ptr[sofa]++;
            }
            // 需要更新沙发队列, 发送sofa customer对barber的quest
            barber_quest_flg = IPC_NOWAIT;
            // quest是非阻塞的, barber_quest一定按时间顺序排列
            if (msgsnd(barber_quest_id, &msg_arg, sizeof(msg_arg), barber_quest_flg) < 0)
                perror("message sent from sofa to barber failed.\n");
        } else if (wait_count < 13){
            // 等待室有空位, 沙发满
            // customer只能进入等待室
            printf("sofa is full, customer %d is waiting in the waiting room\n", id);
            wait_count++;
            // 发送customer对sofa的quest
            sofa_quest_flg = IPC_NOWAIT;
            // quest非阻塞, 发送quest, 这个quest在队列中一定按时间顺序排列了         
            msgsnd(sofa_quest_id, &msg_arg, sizeof(msg_arg), sofa_quest_flg);
        } else
            // 等待室和沙发都满了, customer离开
            printf("waiting room is full, customer %d leaves\n", id);

        // 新customer安顿好了, 处理老customer
        barber_respond_flg = IPC_NOWAIT;
        // sofa接收barber发来的respond, 看是否理发结束, msgtype=0, 接收第1条消息即可, sofa上的人数--
        if (msgrcv(barber_respond_id, &msg_arg, sizeof(msg_arg), 0, barber_respond_flg) > 0)
            buff_ptr[sofa]--;
        // waiting room接收sofa发来的respond, 看sofa是否有空, waiting room的人数--
        sofa_respond_flg = IPC_NOWAIT;
        if (msgrcv(sofa_respond_id, &msg_arg,sizeof(msg_arg), 0, sofa_respond_flg) > 0) {
            printf("customer %d moves to sofa from the waiting room\n", msg_arg.mid);
            wait_count--;
            buff_ptr[sofa]++;
        }
    }

    return EXIT_SUCCESS;
}
