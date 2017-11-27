#include <stdio.h>
#include <unistd.h>

#include "uThread.h"

/*
	This program is an example of how to write a thread safe program using uThread
*/

uthread_key_t *key;

void th1() {
	int x = 10;
	uthread_set_key(key, (void*)&x);
	for(int i = 0; i < 50; i++) {
		int* newx = (int*)uthread_get_key(key);
		(*newx)++;
		uthread_set_key(key, (void *)newx);
	}
	int* newx = uthread_get_key(key);
	printf("X is %d after th1\n", *newx);
	fflush(stdout);
}


void th2() {
	int x = 10;
	uthread_set_key(key, (void*)&x);
	for(int i = 0; i < 50; i++) {
		int* newx = (int*)uthread_get_key(key);
		(*newx)--;
		uthread_set_key(key, (void *)newx);
	}
	int* newx = uthread_get_key(key);
	printf("X is %d after th2\n", *newx);
	fflush(stdout);
}

int main() {
	uthread_init(3);
	uthread_key_create(key);
	uthread_create(th1);
	uthread_create(th2);
	uthread_exit();
}