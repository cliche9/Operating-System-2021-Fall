/* Filename: reader.c
 * Function: 建立并模拟读者进程
 */

#include "ipc.h"

int main(int argc, char *argv[]) {
    int rate = 0, i = 0;
    Msg_buf msg_arg;

    // 进程睡眠时间
    if (argv[1] != NULL)
        rate = atoi(argv[1]);
    else 
        rate = 3;
    // 附加一个要读内容的共享内存
    buff_key = 101;
    buff_num = STRSIZE + 1;
    shm_flg = IPC_CREAT | 0644;
    buff_ptr = (char *)set_shm(buff_key, buff_num, shm_flg);
    // 联系一个请求消息队列
    quest_flg = IPC_CREAT | 0644;
    quest_key = 201;
    quest_id = set_msq(quest_key, quest_flg);
    // 联系一个响应消息队列
    respond_flg = IPC_CREAT | 0644;
    respond_key = 202;
    respond_id = set_msq(respond_key, respond_flg);
    // 循环请求读
    msg_arg.mid = getpid();

    while (1) {
        // quest
        msg_arg.mtype = READERQUEST;
        msgsnd(quest_id, &msg_arg, sizeof(msg_arg), 0);
        printf("%d reader quest\n", msg_arg.mid);
        // block: wait for admission
        msgrcv(respond_id, &msg_arg, sizeof(msg_arg), msg_arg.mid, 0);
        printf("%d reading: %s\n", msg_arg.mid, buff_ptr);
        sleep(rate);
        // read finished
        msg_arg.mtype = FINISHED;
        msgsnd(quest_id, &msg_arg, sizeof(msg_arg), quest_flg);
    }

    return EXIT_SUCCESS;
}