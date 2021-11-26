/*
 * Filename : track.h
 * Function : 声明 IPC 机制的函数原型和火车管程类
*/
#define NORTH 0
#define SOUTH 1
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
using namespace std;

/* 信号量控制用的共同体 */
typedef union semuns {
    int val; 
} Sem_uns;

// 铁路的 3 个状态（思考、饥俄、就餐）
enum Direction {
    north, south
};

// 铁路管程中使用的信号量
class Sema {
public:
    Sema(int id);
    ~Sema();
    int sem_wait();     // 信号量加 1
    int sem_signal();   // 信号量减 1
private:
    int sem_id;         // 信号量标识符
};

// 铁路管程中使用的锁
class Lock {
public:
    Lock(Sema *lock);
    ~Lock();
    void lock();
    void unlock();
private:
    Sema *sema;         // 锁使用的信号量
};

// 铁路管程中使用的条件变量
class Condition {
public:
    Condition(Direction *direction, Sema *north, Sema *south, int *trackCount, int *waitCount);
    ~Condition();
    void wait(Lock *lock, int i, int direction);    // 条件变量阻塞操作
    void signal(int direction);                     // 条件变量唤醒操作
private:
    Sema *east_sema;                // 铁路信号量
    Sema *west_sema;                
    int *waitCount;
    int *trackCount;
    Direction *currentDirection;
};

// 铁路管程的定义
class track {
public:
    track(int rate);                // 管程构造函数
    ~track();
    void trackIn(int trainId, int dirction);      // 进入铁路
    void trackOut(int trainId);     // 离开铁路
    // 建立或获取 ipc 信号量的一组函数的原型说明
    int get_ipc_id(char *proc_file, key_t key);
    int set_sem(key_t sem_key, int sem_val, int sem_flag);
    // 创建共享内存，放铁路状态
    char *set_shm(key_t shm_key, int shm_num, int shm_flag);
private:
    int rate;                       // 控制执行速度
    int maxOneDirection;            // 单方向最多行驶个数
    int *trackCount;                // 铁路上的车辆数
    int *waitCount;
    Lock *lock;                     // 控制互斥进入管程的锁
    Condition *trackCondition;      // 控制铁路状态的条件变量
    Direction *currentDirection;    // 铁路上列车当前的状态
};