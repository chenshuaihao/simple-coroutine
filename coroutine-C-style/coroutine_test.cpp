// Copyright 2020, Chen Shuaihao.
//
// Author: Chen Shuaihao
//
// -----------------------------------------------------------------------------
// File: coroutine_test.cpp
// -----------------------------------------------------------------------------
//
// ref: ucontext-人人都可以实现的简单协程库
// link: https://blog.csdn.net/qq910894904/article/details/41911175
//

// ucontext例子
/*
#include <ucontext.h>
#include <unistd.h>

#include <iostream>

using namespace std;

int main() {
	ucontext_t context;

	getcontext(&context);
	cout << "hello" << endl;
	sleep(1);
	setcontext(&context);
	return 0;
}
*/

/*
#include <ucontext.h>
#include <stdio.h>
 
void func1(void * arg)
{
    puts("1");
    puts("11");
    puts("111");
    puts("1111"); 
}

void context_test()
{
    char stack[1024];
    ucontext_t child,main;
 
    getcontext(&child); //获取当前上下文
    child.uc_stack.ss_sp = stack;//指定栈空间
    child.uc_stack.ss_size = sizeof(stack);//指定栈空间大小
    child.uc_stack.ss_flags = 0;
    child.uc_link = &main;//设置后继上下文
	child.uc_link = NULL;//设置后继上下文

	puts("makecontext");
    makecontext(&child,(void (*)(void))func1,0);//修改上下文指向func1函数
 
	puts("swapcontext");
	swapcontext(&main,&child);//切换到child上下文，保存当前上下文到main
    puts("main");//如果设置了后继上下文，func1函数指向完后会返回此处

}
 
int main()
{
    context_test(); 
    return 0;
}
*/

#include <time.h>
#include <stdio.h>

#include <iostream>

#include "./coroutine.h"

using namespace std;

void func1(void *arg) {
	cout << "func1...1" << endl;
	cout << "func1...2" << endl;
	return ;
}

void func2(void *arg) {
	cout << "func2...1" << endl;
	uthread_yield(*(schedule_t *)arg);
	cout << "func2...2" << endl;
	return ;
}

void func3(void *arg) {
	cout << "func3...1" << endl;
	uthread_yield(*(schedule_t *)arg);
	cout << "func3...2" << endl;
	return ;
}

void schedule_main() {
	schedule_t sche;
	int id1 = uthread_create(sche, func2, &sche);
	int id2 = uthread_create(sche, func3, &sche);
	while(!schedule_finished(sche)) {
		uthread_resume(sche, id1);
		uthread_resume(sche, id2);
	}
	cout << "schedule_main end" << endl;
}

//测试性能
void coroutine_switch_performance_test_func(void *arg) {
	int i = 0;	
	//百万次上下文切换测试
	while(i < 1000000) {
		uthread_yield(*((schedule_t*)arg));
		++i;
	}
}

void coroutine_performance_test() {
	schedule_t sche;
	int id = uthread_create(sche, coroutine_switch_performance_test_func, &sche);
	clock_t start, end;
	start = clock();
	while(!schedule_finished(sche)) {
		uthread_resume(sche, id);
	}
	end = clock();
	printf("Running Time：%lf ms\n", (double)(end - start)/CLOCKS_PER_SEC);
}

int main() {
	schedule_main();
	coroutine_performance_test();
	return 0;
}