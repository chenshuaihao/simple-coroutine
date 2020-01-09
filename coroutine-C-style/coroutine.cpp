// Copyright 2020, Chen Shuaihao.
//
// Author: Chen Shuaihao
//
// -----------------------------------------------------------------------------
// File: coroutine.cpp
// -----------------------------------------------------------------------------
//
// ref: ucontext-人人都可以实现的简单协程库
// link: https://blog.csdn.net/qq910894904/article/details/41911175
//

#include "./coroutine.h"

#include <iostream>

using namespace std;

int uthread_create(schedule_t &schedule, Func func, void *arg) {
	int i = 0;
	for(i = 0; i < schedule.threads.size(); ++i) {
		if(schedule.threads[i].state == FREE) {
			break;
		}
	}

	if(i == schedule.threads.size())
		schedule.threads.push_back(uthread_t());
	
	uthread_t *puth = &schedule.threads[i];
	puth->state = RUNNABLE;
	puth->func = func;
	puth->arg = arg;

	getcontext(&puth->ctx);
	puth->ctx.uc_stack.ss_sp = puth->stack;
	puth->ctx.uc_stack.ss_size = DEFAULT_STACK_SIZE;
	puth->ctx.uc_stack.ss_flags = 0;
	puth->ctx.uc_link = &schedule.mainctx;

	
	makecontext(&puth->ctx, (void (*)(void))(uthread_func), 1, schedule);
	
	//schedule.running_thread = i;
	//swapcontext(&schedule.mainctx, &puth->ctx);
	
	return i;
}


void uthread_yield(schedule_t &schedule) {
	if(schedule.running_thread != -1) {
		uthread_t *puth = &schedule.threads[schedule.running_thread];
		puth->state = SUSPEND;
		schedule.running_thread = -1;

		//保存当前协程的上下文，之后恢复后继续执行
		swapcontext(&puth->ctx, &schedule.mainctx);
	}
}


void uthread_resume(schedule_t &schedule, int id) {
	if(id < 0 || id >= schedule.threads.size()) {
		return ;
	}
	if(id == schedule.running_thread) {
		return ;
	}
	
	switch (schedule.threads[id].state)
	{
		case SUSPEND:
			schedule.running_thread = id;
			schedule.threads[id].state = RUNNING;
			swapcontext(&schedule.mainctx, &schedule.threads[id].ctx);
			break;
		case RUNNABLE:
			//创建协程时没有立即执行，可以用这里来执行
			schedule.running_thread = id;
			swapcontext(&schedule.mainctx, &schedule.threads[id].ctx);
		default:
			break;
	}
}


int schedule_finished(const schedule_t &schedule) {
	if(schedule.running_thread != -1) {
		return 0;
	} else {
		for(int i = 0; i < schedule.threads.size(); ++i) {
			if(schedule.threads[i].state != FREE) {
				return 0;
			}
		}
	}
	return 1;
}

void uthread_func(schedule_t &schedule) {
	int id = schedule.running_thread;
	if(id != -1) {
		uthread_t *puth = &schedule.threads[id];
		puth->state = RUNNING;
		puth->func(puth->arg);
		puth = &schedule.threads[id]; //bugfix 没有这一句会段错误或者死循环，因为
		//这里可能会由于threads 以2倍扩展分配新空间，导致puth指向旧的内存空间，然后swapcontext段错误
		puth->state = FREE;
		schedule.running_thread = -1;
	}
}