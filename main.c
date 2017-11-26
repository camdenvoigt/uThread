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

int main()
{
	uthread_init(1);
	uthread_create(th1);
	uthread_create(th2);
	uthread_create(th3);
	uthread_exit();
}