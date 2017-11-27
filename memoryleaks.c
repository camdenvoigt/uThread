#include <stdio.h>
#include <stdlib.h>

#include "uThread.h"

/*
	This is a very simple program to demonstrate the uThread does not leak memory.
	To see this run this program in valgrind. 
*/

void th1() {
	printf("Thread 1: enter\n");
	for(int i = 0; i < 3; i++) {
		sleep(1);
		printf("Thread 1: yield\n");
		uthread_yield();
	}
	printf("Thread 1: exit\n");
	uthread_exit();
}

void th2() {
	printf("Thread 2: enter\n");
	for(int i = 0; i < 3; i++) {
		sleep(2);
		printf("Thread 2: yield\n");
		uthread_yield();
	}
	printf("Thread 2: exit\n");
	uthread_exit();
}

int main() {
	uthread_init(1);
	uthread_create(th1);
	uthread_create(th2);
	uthread_exit();
}