#include "uThread.h"

int x = 10;

th1() {
	for(int i = 0; i < 50; i++) {
		x++;
	}
}

th2() {
	for(int i = 0; i < 50; i++) {
		x--;
	}
}

int main() {
	uthread_init(2);
	uthread_create(th1);
	uthread_create(th2);
	uthread_exit();
}