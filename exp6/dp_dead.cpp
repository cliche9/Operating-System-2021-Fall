/*
 * Filename : dp.cc
 * copyright : (C) 2006 by zhonghonglie
 * Function : 哲学家就餐问题的模拟程序
 */
#include "dp_dead.h"
using std::cout;
using std::endl;

/*
 * get_ipc_id() 从 /proc/sysvipc/ 文件系统中获取 IPC 的 id 号
 * pfile: 对应 /proc/sysvipc/ 目录中的 IPC 文件分别为
 * msg- 消息队列 ,sem- 信号量 ,shm- 共享内存
 * key: 对应要获取的 IPC 的 id 号的键值
 */
int get_ipc_id(char *proc_file, key_t key) {
    #define BUFSZ 256
    
    FILE *pf;
    int i, j;
    char line[BUFSZ], colum[BUFSZ];
    if ((pf = fopen(proc_file, "r")) == NULL) {
        perror("Proc file not open");
        exit(EXIT_FAILURE);
    }

    fgets(line, BUFSZ, pf);
    while (!feof(pf)) {
        i = j = 0;
        
        fgets(line, BUFSZ, pf);
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
    fclose(pf);
    return -1;
}

/*
 * set_sem 函数建立一个具有 n 个信号量的信号量
 * 如果建立成功，返回 一个信号量的标识符 sem_id
 * 输入参数：
 * sem_key 信号量的键值
 * sem_val 信号量中信号量的个数
 * sem_flag 信号量的存取权限
 */
int set_sem(key_t sem_key, int sem_val, int sem_flg) {
    int sem_id; 
    Sem_uns sem_arg;
    // 测试由 sem_key 标识的信号量是否已经建立
    if ((sem_id = get_ipc_id("/proc/sysvipc/sem", sem_key)) < 0) {
        //semget 新建一个信号量 , 其标号返回到 sem_id
        if ((sem_id = semget(sem_key, 1, sem_flg)) < 0) {
            perror("semaphore create error");
            exit(EXIT_FAILURE);
        }
    }
    // 设置信号量的初值
    sem_arg.val = sem_val;
    if (semctl(sem_id, 0, SETVAL, sem_arg) < 0) {
        perror("semaphore set error");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}

Sema::Sema(int id) {
    sem_id = id;
}

Sema::~Sema() { }

/*
 * 信号量上的 down/up 操作
 * semid: 信号量数组标识符
 * semnum: 信号量数组下标
 * buf: 操作信号量的结构
 */
int Sema::sem_wait() {
    cout << "enter sem wait\n";
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("sem wait error ");
        exit(EXIT_FAILURE);
    }
    cout << "semop in\n";
    return EXIT_SUCCESS;
}

int Sema::sem_signal() {
    Sem_uns arg;
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("sem signal error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

chopStick::chopStick(int id) {
    this->id = id + 1;
    int ipc_flg = IPC_CREAT | 0644;
    int sem_key = 121;
    int sem_val = 1;
    int sem_id;

    if ((sem_id = set_sem(sem_key + id * 10, 1, ipc_flg)) < 0) {
        perror("Semaphor create error");
        exit(EXIT_FAILURE);
    }
    
    Sema *sema = new Sema(sem_id);
}

void chopStick::pickup() {
    cout << "enter pick up\n";
    sema->sem_wait();
}

void chopStick::putdown() {
    sema->sem_signal();
}

int chopStick::getId() {
    return this->id;
}

dp::dp(int id, int rate, chopStick *left, chopStick *right) {
    this->id = id;
    this->rate = rate;
    this->left = left;
    this->right = right;
}

dp::~dp() {}

void dp::run() {
    while (true) {
        int pid = getpid();
        cout << "pid: " << pid << " thinking\n";
        if (this->id % 2) {
            left->pickup();
            cout << "pid: " << pid << " pick up left chopstick: " << left->getId() << "\n";
            sleep(rate);
            right->pickup();
            cout << "pid: " << pid << " pick up right chopstick " << right->getId() << "\n";
            sleep(rate);
            cout << "pid: " << pid << " eating\n";
            left->putdown();
            cout << "pid: " << pid << " put down left chopstick " << left->getId() << "\n";
            sleep(rate);
            right->putdown();
            cout << "pid: " << pid << " put down right chopstick " << right->getId() << "\n";
            sleep(rate);
        } else {
            right->pickup();
            cout << "pid: " << pid << " pick up right chopstick " << right->getId() << "\n";
            sleep(rate);
            left->pickup();
            cout << "pid: " << pid << " pick up left chopstick: " << left->getId() << "\n";
            sleep(rate);
            cout << "pid: " << pid << " eating\n";
            right->putdown();
            cout << "pid: " << pid << " put down right chopstick " << right->getId() << "\n";
            sleep(rate);
            left->putdown();
            cout << "pid: " << pid << " put down left chopstick " << left->getId() << "\n";
            sleep(rate);
        }
    }
}


int main(int argc, char *argv[]) {
    int rate = (argc > 1) ? atoi(argv[1]) : 3;
    int sum = 5;
    chopStick *chopSticks[sum];
    
    for (int i = 0; i < sum; i++) {
        chopSticks[i] = new chopStick(i);
    }
    int pid[5];

    pid[0] = fork(); // 建立第一个哲学家进程
    if (pid[0] < 0) {
        perror("p1 create error");
        exit(EXIT_FAILURE);
    } else if (pid[0] == 0) {
        dp dp(1, rate, chopSticks[0], chopSticks[1]);
        dp.run();
        cout << "dp1 end\n";
    }

    pid[1] = fork(); // 建立第二个哲学家进程
    if (pid[1] < 0) {
        perror("p2 create error");
        exit(EXIT_FAILURE);
    } else if (pid[1] == 0) {
        dp dp(2, rate, chopSticks[1], chopSticks[2]);
        dp.run();
        cout << "dp2 end\n";
    }

    pid[2] = fork(); // 建立第三个哲学家进程
    if (pid[2] < 0) {
        perror("p3 create error");
        exit(EXIT_FAILURE);
    } else if (pid[2] == 0) {
        dp dp(3, rate, chopSticks[2], chopSticks[3]);
        dp.run();
        cout << "dp3 end\n";
    }

    pid[3] = fork(); // 建立第四个哲学家进程
    if (pid[3] < 0) {
        perror("p4 create error");
        exit(EXIT_FAILURE);
    } else if (pid[3] == 0) {
        dp dp(4, rate, chopSticks[3], chopSticks[4]);
        dp.run();
        cout << "dp4 end\n";
    }

    pid[4] = fork(); // 建立第五个哲学家进程
    if (pid[4] < 0) {
        perror("p5 create error");
        exit(EXIT_FAILURE);
    } else if (pid[4] == 0) {
        dp dp(5, rate, chopSticks[4], chopSticks[0]);
        dp.run();
        cout << "dp5 end\n";
    }
}