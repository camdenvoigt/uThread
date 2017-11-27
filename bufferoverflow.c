/*
	This is a sipmle program that shows that bufferoverflow cannot be done using uThread.
*/

/*
	test function that creates an int i and sees if another thread modifys it
*/
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

/*
	function that attempts to modify an integer in another thread
*/
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
	uthread_create(th4);
	uthread_create(th5);
	uthread_exit();
}
