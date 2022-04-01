/*
* Filename : ipc.c
* copyright : (C) 2006 by zhonghonglie * Function : 一组建立 IPC 机制的函数
*/
#include "ipc.h"

/*
* get_ipc_id() 从/proc/sysvipc/文件系统中获取 IPC 的 id 号
* pfile: 对应/proc/sysvipc/目录中的 IPC 文件分别为
* msg-消息队列,sem-信号量,shm-共享内存
* key: 对应要获取的 IPC 的 id 号的键值 
*/
int get_ipc_id(char *proc_file, key_t key) {
    FILE *pf;
    int i, j;
    char line[BUFFERSIZE], colum[BUFFERSIZE];
    // 打开IPC文件
    if ((pf = fopen(proc_file, "r")) == NULL) {
        perror("Proc file not open"); 
        exit(EXIT_FAILURE);
    }
    // 持续读取IPC文件内容
    fgets(line, BUFFERSIZE, pf);
    while (!feof(pf)) {
        // 持续读取, 直到读取到IPC对应的ID号
        i = j = 0;
        fgets(line, BUFFERSIZE, pf);
        while (line[i] == ' ')
            i++;
        while (line[i] != ' ')
            colum[j++] = line[i++];
        colum[j] = '\0';
        if (atoi(colum) != key)
            continue;
        j = 0;
        while (line[i] == ' ') 
            i++;
        while (line[i] != ' ')
            colum[j++] = line[i++]; 
        colum[j] = '\0';
        i = atoi(colum);
        fclose(pf);
        return i;
    }
    // 没读取到, 返回-1
    fclose(pf);
    return -1;
}

/*
* 信号量上的 P/V 操作 
* semid: 信号量数组标识符 
* semnum: 信号量数组下标 
* buf: 操作信号量的结构
*/
int P(int sem_id) {
    // P操作
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("P error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int V(int sem_id) {
    // V操作
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO; 
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("V error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS; 
}

/*
* set_sem 函数建立一个具有 n 个信号量的信号量
* 如果建立成功，返回 一个信号量数组的标识符 sem_id
* 输入参数:
* sem_key 信号量数组的键值
* sem_val 信号量数组中信号量的个数
* sem_flag 信号量数组的存取权限
*/
int set_sem(key_t sem_key, int sem_val, int sem_flg) {
    int sem_id;
    Sem_uns sem_arg;
    // 测试由 sem_key 标识的信号量数组是否已经建立
    if ((sem_id = get_ipc_id("/proc/sysvipc/sem", sem_key)) < 0) {
        // semget 新建一个信号量,其标号返回到 sem_id
        if ((sem_id = semget(sem_key, 1, sem_flg)) < 0) {
            perror("semaphore create error");
            exit(EXIT_FAILURE); 
        }
        // 设置信号量的初值 
        sem_arg.val = sem_val;
        // 具体是什么意思？
        if (semctl(sem_id, 0, SETVAL, sem_arg) < 0) {
            perror("semaphore set error");
            exit(EXIT_FAILURE);
        }
    }
    return sem_id; 
}

/*
* remove_sem 函数将信号量集从内存中删除, 并唤醒因调用semop()而阻塞的进程
* 如果建立成功，返回 一个信号量数组的标识符 sem_id
* 输入参数:
* sem_key 信号量数组的键值
* sem_val 信号量数组中信号量的个数
* sem_flag 信号量数组的存取权限

int remove_sem(key_t sem_key) {
    int sem_id;
    Sem_uns sem_arg;
    // 测试由 sem_key 标识的信号量数组是否已经建立
    if ((sem_id = get_ipc_id("/proc/sysvipc/sem", sem_key)) < 0) {
        perror("semaphore not exists");
    } else {
        if (semctl(sem_id, 0, IPC_RMID) < 0) {
            perror("semaphore remove error");
            exit(EXIT_FAILURE);
        }
    }
    return sem_id;
}
*/

/*
* set_shm 函数建立一个具有 n 个字节 的共享内存区
* 如果建立成功，返回 一个指向该内存区首地址的指针 shm_buf
* 输入参数:
* shm_key 共享内存的键值
* shm_val 共享内存字节的长度
* shm_flag 共享内存的存取权限
*/
char *set_shm(key_t shm_key, int shm_num, int shm_flg) {
    int i, shm_id;
    char *shm_buf;
    // 如果由 shm_key 标识的共享内存区未建立
    if ((shm_id = get_ipc_id("/proc/sysvipc/shm", shm_key)) < 0) {
        // shmget 新建 一个长度为 shm_num 字节的共享内存,其标号返回 shm_id
        if ((shm_id = shmget(shm_key, shm_num, shm_flg)) < 0) {
            perror("shareMemory set error");
            exit(EXIT_FAILURE);
        }
        // shmat 将由 shm_id 标识的共享内存附加给指针 shm_buf 
        if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) { 
            perror("get shareMemory error"); 
            exit(EXIT_FAILURE); 
        }
        for (i = 0; i < shm_num; i++)
            // 初始共享内存区为 0
            shm_buf[i] = 0;
    }
    // 如果shm_key 标识的共享内存区已经建立, 将由 shm_id 标识的共享内存附加给指针 shm_buf 
    if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
        perror("get shareMemory error");
        exit(EXIT_FAILURE);
    }

    return shm_buf; 
}

/*
* set_msq 函数建立一个消息队列
* 如果建立成功，返回 一个消息队列的标识符 msq_id
* 输入参数:
* msq_key 消息队列的键值
* msq_flag 消息队列的存取权限 
*/
int set_msq(key_t msq_key, int msq_flg) {
    int msq_id;
    // 如果由 msq_key 标识的消息队列未建立
    if ((msq_id = get_ipc_id("/proc/sysvipc/msg",msq_key)) < 0) {
        // msgget 新建一个消息队列, 标号返回到 msq_id
        if ((msq_id = msgget(msq_key, msq_flg)) < 0) {
            perror("messageQueue set error");
            exit(EXIT_FAILURE);
        }
    }
    return msq_id; 
}