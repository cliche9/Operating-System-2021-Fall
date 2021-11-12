/* Filename: control.c
 * Function: 建立并模拟控制者进程
 */

#include "ipc.h"

int main(int argc, char *argv[]) {
    int rate = 0, i = 0;
    int w_mid = 0;
    int count = MAXVAL;         // count > 读者个数
    Msg_buf msg_arg;            // 消息结构体, 存储消息type和id
    struct msqid_ds msg_inf;    // 内核中用于记录消息的详细信息
    // sharing memory
    buff_key = 101;
    buff_num = STRSIZE + 1;
    shm_flg = IPC_CREAT | 0644;
    buff_ptr = (char *)set_shm(buff_key, buff_num, shm_flg);
    for (i = 0; i < STRSIZE; i++)
        buff_ptr[i] = 'A';
    buff_ptr[i] = '\0';

    // querying message queue
    quest_flg = IPC_CREAT | 0644;
    quest_key = 201;
    quest_id = set_msq(quest_key, quest_flg);

    // responding message queue
    respond_flg = IPC_CREAT | 0644;
    respond_key = 202;
    respond_id = set_msq(respond_key, respond_flg);

    // 控制进程: 接收和响应读写者消息
    printf("Wait quest\n");
    while (1) {
        // count > 0: 空闲, 可以接收读写请求
        // 此时只有可能读者在读
        if (count > 0) {
            quest_flg = IPC_NOWAIT; // 非阻塞方式接受请求信息
            if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), FINISHED, quest_flg) >= 0) {
                // 有读者完成
                count++;
                printf("%d reader finished\n", msg_arg.mid);
            } else if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), READERQUEST, quest_flg) >= 0) {
                // 读者请求
                count--;
                // 为什么这样用？
                msg_arg.mtype = msg_arg.mid;
                msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
                printf("%d quest read\n", msg_arg.mid);
            } else if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), WRITERQUEST, quest_flg) >= 0) {
                // 读者请求
                count -= MAXVAL;
                // 为什么这样用？
                msg_arg.mtype = msg_arg.mid;
                msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
                printf("%d quest write\n", msg_arg.mid);
            }
        } else if (count == 0) {
            // count == 0, 写者正在写, 等待完成
            // 阻塞方式接收消息, 等待写完成
            // 写完成消息, 阻塞等待完成
            msgrcv(quest_id, &msg_arg, sizeof(msg_arg), FINISHED, 0);
            count = MAXVAL;
            printf("%d write finished\n", msg_arg.mid);
            // 写完有读请求, 这里还需要写在这里吗, 是否和之前的重复了？
            /*
            if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), READERQUEST, quest_flg) >= 0) {
                count--;
                msg_arg.mtype = msg_arg.mid;
                msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
                printf("%d quest read\n", msg_arg.mid);
            }
            */
        } else if (count < 0) {
            // count < 0, 处理读者, 将其完成, 使得count = 0
            msgrcv(quest_id, &msg_arg, sizeof(msg_arg), FINISHED, 0);
            count++;
            printf("%d reader finished\n", msg_arg.mid);
        }
    }

    return EXIT_SUCCESS;
}