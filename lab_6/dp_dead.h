/*
 * Filename : dp.h
 * copyright : (C) 2006 by zhonghonglie
 * Function : 声明 IPC 机制的函数原型和哲学家管程类
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>

/* 信号量控制用的共同体 */
typedef union semuns {
    int val; 
} Sem_uns;

// 哲学家的 3 个状态（思考、饥俄、就餐）
enum State {
    thinking, hungry, eating 
};

// 信号量
class Sema {
public:
    Sema(int id);
    ~Sema();
    int sem_wait();     // 信号量 -1
    int sem_signal();   // 信号量 +1
private:
    int sem_id;         // 信号量标识符
};

// 筷子
class chopStick {
public:
    chopStick(int id);
    ~chopStick();
    void pickup();             // 获取筷子
    void putdown();            // 放下筷子
    int getId();
private:
    int id;
    Sema *sema;
};

// 哲学家
class dp {
public:
    dp(int id, int rate, chopStick *left, chopStick *right);   // 管程构造函数
    ~dp();
    void run();
private:
    int id;
    int rate;                       // 控制执行速度
    chopStick *left;
    chopStick *right;
};
