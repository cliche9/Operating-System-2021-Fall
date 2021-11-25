/*
 * Filename : track.cpp
 * Function : 铁路单行道问题的模拟程序
 */
#include "track.h"
#include <random>
using namespace std;

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
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("sem wait error ");
        exit(EXIT_FAILURE);
    }
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

/* * 用于哲学家管程的互斥执行 */
Lock::Lock(Sema *s) {
    sema = s;
}

Lock::~Lock() {}

// 上锁
void Lock::lock() {
    sema->sem_wait();
}

// 开锁
void Lock::unlock() {
    sema->sem_signal();
}

// 用于哲学家就餐问题的条件变量
Condition::Condition(Direction *direction, Sema *east, Sema *west, int *trackCount, int *waitCount) {
    currentDirection = direction;
    trackCount = trackCount;
    waitCount = waitCount;
    east_sema = east;
    west_sema = west;
}

/*
 * 当前铁路可以通行, 则占据铁路, 使其无法通行
 * 否则睡眠，等待条件成立
 */
void Condition::wait(Lock *lock, int i, int direction) {
    if (direction == east) {
        cout << "train " << i << " : " << getpid() << " quest to track in from east\n";
        lock->unlock();         // 开锁
        east_sema->sem_wait();  // 等待
        lock->lock();           // 上锁
    } else if (direction == west) {
        cout << "train " << i << " : " << getpid() << " quest to track in from west\n";
        lock->unlock();         // 开锁
        west_sema->sem_wait();  // 等待
        lock->lock();           // 上锁
    }
}

/*
 * 当前铁路可以通行
 * 否则什么也不做
 */
void Condition::signal(int direction) {
    if (direction == east)
        east_sema->sem_signal();
    else if (direction == west)
        west_sema->sem_signal();
}

Condition::~Condition() {}

/*
 * get_ipc_id() 从 /proc/sysvipc/ 文件系统中获取 IPC 的 id 号
 * pfile: 对应 /proc/sysvipc/ 目录中的 IPC 文件分别为
 * msg- 消息队列 ,sem- 信号量 ,shm- 共享内存
 * key: 对应要获取的 IPC 的 id 号的键值
 */
int track::get_ipc_id(char *proc_file, key_t key) {
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
int track::set_sem(key_t sem_key, int sem_val, int sem_flg) {
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

/* set_shm 函数建立一个具有 n 个字节 的共享内存区
* 如果建立成功，返回 一个指向该内存区首地址的指针 shm_buf
* 输入参数：
* shm_key 共享内存的键值
* shm_val 共享内存字节的长度
* shm_flag 共享内存的存取权限
*/
char *track::set_shm(key_t shm_key, int shm_num, int shm_flg) {
    int i, shm_id;
    char *shm_buf;
    // 测试由 shm_key 标识的共享内存区是否已经建立
    if ((shm_id = get_ipc_id("/proc/sysvipc/shm", shm_key)) < 0) {
        //shmget 新建 一个长度为 shm_num 字节的共享内存
        if ((shm_id = shmget(shm_key, shm_num, shm_flg)) < 0) {
            perror("shareMemory set error");
            exit(EXIT_FAILURE);
        }
        //shmat 将由 shm_id 标识的共享内存附加给指针 shm_buf
        if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
            perror("get shareMemory error");
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < shm_num; i++)
            shm_buf[i] = 0; // 初始为 0
    }
    // 共享内存区已经建立 , 将由 shm_id 标识的共享内存附加给指针 shm_buf
    if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
        perror("get shareMemory error");
        exit(EXIT_FAILURE);
    }
    return shm_buf;
}

// 哲学家就餐问题管程构造函数
track::track(int r) {
    int ipc_flg = IPC_CREAT | 0644;
    int shm_key = 220;
    int shm_num = 1;
    int sem_key = 120;
    int sem_val = 0;
    int sem_id;
    Sema *sema;
    Sema *east_sem, *west_sem;

    rate = r;
    
    // 建立一个初值为 1 的用于锁的信号量
    if ((sem_id = set_sem(sem_key++, 1, ipc_flg)) < 0) {
        perror("Semaphor create error");
        exit(EXIT_FAILURE);
    }
    sema = new Sema(sem_id);
    lock = new Lock(sema);

    if ((currentDirection = (Direction *)set_shm(shm_key++, shm_num, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    if ((trackCount = (int *)set_shm(shm_key++, shm_num, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    if ((waitCount = (int *)set_shm(shm_key++, 2, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    sem_val = 0;
    *currentDirection = east;
    if ((sem_id = set_sem(sem_key++, sem_val, ipc_flg)) < 0) {
        perror("Semaphor create error ");
        exit(EXIT_FAILURE);
    }
    east_sem = new Sema(sem_id);
    if ((sem_id = set_sem(sem_key++, sem_val, ipc_flg)) < 0) {
        perror("Semaphor create error ");
        exit(EXIT_FAILURE);
    }
    west_sem = new Sema(sem_id);
    trackCondition = new Condition(currentDirection, east_sem, west_sem, trackCount, waitCount);
    maxOneDirection = 6;
    *trackCount = 0;
    waitCount[EAST] = waitCount[WEST] = 0;
}

void track::trackIn(int i, int direction) {
    lock->lock();
    if (*currentDirection != direction || *trackCount >= maxOneDirection) {
        waitCount[direction]++;
        trackCondition->wait(lock, i, direction);
        waitCount[direction]--;
    }
    
    (*trackCount)++;
    lock->unlock();

    string dir = (direction == east) ? "east" : "west";
    if (*currentDirection == east)
        cout << "east: " << *trackCount << ", west: 0\n"; 
    else if (*currentDirection == west)
        cout << "east: 0, west: " << *trackCount << endl; 

    cout << "train " << i << " : " << getpid() << " is on track in direction of " << dir << "\n";
    sleep(rate);
}

void track::trackOut(int i) {
    lock->lock();
    
    (*trackCount)--;
    cout << "train " << i << " : " << getpid() << " has left track\n";
    if (*currentDirection == east)
        cout << "east: " << *trackCount << ", west: 0\n"; 
    else if (*currentDirection == west)
        cout << "east: 0, west: " << *trackCount << endl;
    
    if (*trackCount == 0) {
        if (*currentDirection == east) {
            if (waitCount[WEST] > 0)
                trackCondition->signal(west);
            *currentDirection = west;
        } else if (*currentDirection == west) {
            if (waitCount[EAST] > 0)
                trackCondition->signal(east);
            *currentDirection = east;
        }
    }

    lock->unlock();

    sleep(rate);
}

track::~track() {
    delete trackCondition;
}

int main(int argc, char *argv[]) {
    int rate = (argc > 1) ? atoi(argv[1]) : 3;
    int maxTrains = 0;
    track theTrack(rate);
    cout << "请输入经过铁轨的总车辆数: ";
    cin >> maxTrains;

    int pid[maxTrains];
    srand(time(nullptr));
    for (int i = 0; i < maxTrains; i++) {
        pid[i] = fork();
        int direction = rand() % 2;
        if (pid[i] < 0) {
            perror("Creating child process fails ");
            exit(EXIT_FAILURE);
        } else if (pid[i] == 0) {
            sleep(rate);
            theTrack.trackIn(i, direction);
            theTrack.trackOut(i);
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < maxTrains; i++) {
        waitpid(pid[i], NULL, 0);
    }

    return EXIT_SUCCESS;
}