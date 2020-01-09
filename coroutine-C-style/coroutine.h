// Copyright 2020, Chen Shuaihao.
//
// Author: Chen Shuaihao
//
// -----------------------------------------------------------------------------
// File: coroutine.h
// -----------------------------------------------------------------------------
//
// ref: ucontext-人人都可以实现的简单协程库
// link: https://blog.csdn.net/qq910894904/article/details/41911175
//

/*
typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  } ucontext_t;

typedef struct sigaltstack
  {
    void *ss_sp;
    int ss_flags;
    size_t ss_size;
  } stack_t;
*/
/*

4个函数：
函数执行控制权转移到ucp
int setcontext(const ucontext_t *ucp);
将当前上下文保存到中ucp
int getcontext(ucontext_t *ucp);
修改上下文入口函数 用于将一个新函数和堆栈，绑定到指定context中
void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...);
保存上下文到oucp 切换到ucp
int swapcontext(ucontext_t *oucp, ucontext_t *ucp);
*/

#include <ucontext.h>
#include <unistd.h>

#include <vector>

#define DEFAULT_STACK_SIZE 1024*8

//协程结构体
typedef void (*Func)(void *arg);

enum ThreadState {FREE, RUNNABLE, RUNNING, SUSPEND};

typedef struct uthread_t {
	ucontext_t ctx;
	Func func;
	void *arg;
	enum ThreadState state; //FREE, RUNNABLE, RUNNING, SUSPEND
	char stack[DEFAULT_STACK_SIZE];
} uthread_t;

// 协程调度器
typedef std::vector<uthread_t> UThread_vector;

typedef struct schedule_t {
	ucontext_t mainctx; //main的上下文
	int running_thread; //正在运行的协程编号
	UThread_vector threads; //协程数组

	schedule_t() : running_thread(-1) {}
} schedule_t;

//创建协程，func是执行函数，加入协程数组中,暂时只创建不执行
int uthread_create(schedule_t &schedule, Func func, void *arg);

//挂起正在执行的协程，切换到main
void uthread_yield(schedule_t &schedule);

//恢复编号id的协程
void uthread_resume(schedule_t &schedule, int id);

//判断所有协程是否执行完毕
int schedule_finished(const schedule_t &schedule);

//协程函数入口
void uthread_func(schedule_t &schedule);
