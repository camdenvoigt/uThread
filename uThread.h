#ifndef uThread
#define uThread

/*
 * 	many2many mapping:
 * 	the library provides untread_init, uthread_create, uthread_yield and uthread_exit
*/

/* THREAD FUNCTIONS */

/*
	This function has to be called before any other functions of the uthread library can be called.
	It initializes the uthread system and specifies the maximum number of kernel threads to be argument numKernelThreads.
	For example, it may establish and initialize a priority ready queue of user-level threads
	(with the amount of time each user-level thread has been mapped to kernel threads as priority number)
	and other data structures.
*/
int uthread_init(int maxNumKernelThreads);

/*
	The calling thread requests the thread library to create a new user-level thread that runs the function func(),
	which is specified as the argument of this function. At the time when this function is called, if less than
	numKernelThreads kernel threads have been active, a new kernel thread is created to execute function func();
	 otherwise, a context of a new user-level thread should be properly created and stored on the priority ready queue.
	 This function returns 0 if succeeds, or -1 otherwise.
*/
int uthread_create(void (*fn)());

/*
	This function is called when the calling user-level thread terminates its execution.
	In response to this call, if no ready user-level thread in the system, the whole process terminates;
	otherwise, a ready user thread with the highest priority should be mapped to the kernel thread to run.
*/
void uthread_exit();

/*
	The calling thread requests to yield the kernel thread to another user-level thread with the same or higher priority
	(note: the priority is based on the time a thread has been mapped to kernel threads). If each ready thread has lower
	priority than this calling thread, the calling thread will continue its running; otherwise, the kernel thread is yielded
	to a ready thread with the highest priority.
*/
int uthread_yield();

/* KEY FUNCTIONS */

/*
	Structure that represents a key for a thread safe value
*/
typedef struct uthread_key {
	void* value;
	struct uthread_key *refKey;
} uthread_key_t;


/*
	Attach a key to every thread value for each thread is set to NULL
*/
void uthread_key_create(uthread_key_t *key);

/*
	Retrives the value assoicated with the given key for the current thread.
*/
void* uthread_get_key(uthread_key_t *key);

/*
	Sets the value assoicated witht he given key for the current thread.
*/
int uthread_set_key(uthread_key_t *key, void *value);

#endif
