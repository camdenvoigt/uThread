#include <stdio.h>
#include <stdlib.h>
#include "uThread.h"

/**** Application code ****/

void th1()
{
	int i;
	for(i=0;i<6;i++){
		printf("Thread 1: run.\n");
		sleep(1);
		printf("Thread 1: yield.\n");
		uthread_yield();
	}
	printf("Thread 1: exit.\n");
	uthread_exit();
}

void th2()
{
	int i;
	for(i=0;i<6;i++){
		printf("Thread 2: run.\n");
		sleep(1);
		printf("Thread 2: yield.\n");
		uthread_yield();
	}
	printf("Thread 2: exit.\n");
	uthread_exit();
}

void th3()
{
	int i;
	for(i=0;i<3;i++){
		printf("Thread 3: run.\n");
		sleep(2);
		printf("Thread 3: yield.\n");
		uthread_yield();
	}
	printf("Thread 3: exit.\n");
	uthread_exit();
}

//test function that creates an int i and sees if another thread modifys it
void th4()
{
	printf("Thread 4: run.\n");
	int i = 5;
	printf("i address %d\n", &i);
	printf("thd4, i = %d\n", i);
	uthread_yield();
	printf("thd4, i = %d\n", i);
	printf("Thread 4: exit.\n");
	uthread_exit();
}

//function that attempts to modify an integer in another thread
void th5()
{
	printf("Thread 5: run.\n");
	int x = 6;
	printf("x address %d\n", &x);
	int *d = &x;
	d = d - (17368 / 4);
	*d = 77;
	printf(" thd 5, x = %d\n", x);
	uthread_yield();
	printf("thd 5, d = %d\n", d);
	printf("Thread 5: exit.\n");
	uthread_exit();
}


int main()
{
	uthread_init(99);
	//uthread_create(th1);
	//uthread_create(th2);
	//uthread_create(th3);
	uthread_create(th4);
	uthread_create(th5);
	uthread_exit();
}
