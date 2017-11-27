#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <semaphore.h> //to use semaphore functions
#include <sys/types.h> //gettid()
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>

#include "uThread.h"

/**** Thread library code ****/

#define STACK_SIZE_BASE 16384

//semaphore used in the library code
sem_t queueMutex;

//max and current number of kernel threads
#define MAX_KTHS 10
int maxNumKThs;
int curNumKThs;
int offset; //the offset size which will be added to the STACK_SIZE_BASE

struct thread_info{
	ucontext_t 		*ucp;
	struct thread_info 	*next;
	unsigned long 		runTime; 
};
struct thread_info *head=NULL, *tail=NULL;

//info about threads that are running
struct kth_info{
	int  state; //0-not used, 1-valid
    pid_t kthID;
    struct thread_info *th;
	unsigned long runTime;
	struct timeval	startTime; //the most recent time when this thread was mapped to a kernel thread
};
struct kth_info kthInfo[MAX_KTHS];

//help functions

/* compare times of two records */
int compareTime(struct timeval v1, struct timeval v2)
{
	if(v1.tv_sec > v2.tv_sec){
		return 1;
	} else if(v1.tv_sec < v2.tv_sec){
		return -1;
	} else if(v1.tv_usec > v2.tv_usec){
		return 1;
	} else if(v1.tv_usec < v2.tv_usec){
		return -1;
	} else {
		return 0;
	}
}

/* gets the elapsed time of a record in milliseconds */
unsigned long elapseTime(struct timeval v)
{
	unsigned long t1=1000000 * v.tv_sec + v.tv_usec;
	struct timeval v1;
	gettimeofday(&v1,NULL);
	unsigned long t2=1000000 * v1.tv_sec + v1.tv_usec;
	return t2-t1;
}

/* takes object form the front of the queue */
struct thread_info *deQueue()
{
	if(head==NULL) return NULL;
	struct thread_info *p=head;
	head=head->next;
	if(head==NULL) tail=NULL;
	return p;
}

/* adds record into the priority queue based on runTime of the record*/
void enQueue(struct thread_info *record)
{
	unsigned long runTime=record->runTime;

	//special case: empty queue
	if(head==NULL){
		record->next=NULL;
		head=tail=record;
		return;
	}

	//find the position in the Q to insert record
	struct thread_info *pre,*cur;
	pre=NULL;
	cur=head;
	while(cur!=NULL && cur->runTime<=runTime){
		pre=cur;
		cur=cur->next;
	}

	if(cur==NULL){
		//insert to the tail
		record->next=NULL;
		tail->next=record;
		tail=record;
	}else{
		//i.e., cur->runTime>runtime ==> insert before cur
		record->next=cur;
		pre->next=record;
	}
}

void thread_info_destroy(struct thread_info * th) {
	free(th->ucp->uc_stack.ss_sp);
	free(th->ucp);
	free(th);
}

int uthread_init(int maxNumKernelThreads)
{
	if(maxNumKernelThreads > 10 || maxNumKernelThreads < 0) {
			printf("invalid number of max threads\n");
		return -1;
	}

	int i;

	//generate a random offset between 0-1023 so that the size of the stack
	//differs between runs. This is done to amke it more difficult to
	//manipulate exact memory in other threads
	int seed = time(NULL);
    srand(seed);
	offset = rand() % 1024;

	printf("uthread_init: enter\n");

	/* init max and cur numbers of kernel threads */
	maxNumKThs = maxNumKernelThreads;
	curNumKThs = 1;

	/* initialie all threads with state 0 and id -1 */
	printf("%d", maxNumKThs);
	for(i = 0; i < maxNumKThs; i++){
		kthInfo[i].state = 0; //not in use
		kthInfo[i].kthID = -1;
	}

	/*
		initialize the library's main thread
		state = 1 means the thread is active
		gets the thread id from system call
	*/
	kthInfo[0].state = 1;
	kthInfo[0].kthID = syscall(SYS_gettid);
	kthInfo[0].th = NULL;

	/*
		init semaphore named queueMutex
		semeaphore pointer defined above
		0 means semaphore isn't shared between processes
		1 means seamphore value is set to 1
	*/
	sem_init(&queueMutex, 0, 1);

	printf("uthread_init: exit\n");
}

int uthread_create(void (*func)())
{
	int i;
	printf("uthread_create: enter\n");

	/* Waiting so only one thread can enter the critical section */
	sem_wait(&queueMutex);

	/* current number of threads is less than max number of threads */
	if(curNumKThs < maxNumKThs) {

		//A new kernel thread can be started immediately

		void *child_stack;
		child_stack=(void *)malloc(STACK_SIZE_BASE + offset);

        /* failed to allocate heap memory */
		if(child_stack==NULL) {
			printf("Fail to allocate heap space for child_stack.\n");
			sem_post(&queueMutex);
			return -1;
		}

		/* move child_stack pointer to end of child_stack memory */
		child_stack += STACK_SIZE_BASE + offset - 1;

		/* create a function pointer that returns an int and takes a void function pointer as an argument*/
		int (*kfunc)(void *arg);

		/* casting func as kfunc type */
		kfunc = (int (*)(void *))func;

		/*
			Creating the new thread that runs kfunc at the start of execution
			and uses child_stack as it's stack memory.
			NOTE: Threads grow downward in linux hence why we passed in the topmost space of the stack.
			Flags
				- CLONE_VM makes new process run in same memory space
				- CLONE_FILES share same file descriptor table
		*/
		pid_t tid = clone(kfunc, child_stack, CLONE_VM|CLONE_FILES, NULL);

		/* creating the new thread failed */
		if(tid == -1){
			printf("Fail to clone a new thread.\n");
			sem_post(&queueMutex);
			return -1;
		}

		/* finds the next kernal thread not in use */
		for(i = 0; i < maxNumKThs; i++) {
			if(kthInfo[i].state == 0) {
				break;
			}
		}

		/* couldn't find the current thread in the thread array */
		if(i == maxNumKThs){
			printf("No available kthInfo record is found - something wrong!\n");
			sem_post(&queueMutex);
			return -1;
		}

		/* set the thread id */
		kthInfo[i].kthID = tid;

		printf("i=%d, tid=%d\n", i, tid);

 		/* set start values for the thread */
		gettimeofday(&kthInfo[i].startTime, NULL);
		kthInfo[i].runTime = 0;
		kthInfo[i].state = 1;

		/* increment current number of threads */
		curNumKThs++;
	} else {

		//construct a thread record

		/* create and allocate a thread_info object */
		struct thread_info *th;
		th = (struct thread_info *) malloc(sizeof(struct thread_info));
		if(th == NULL){
			printf("Fail to allocate space for th.\n");
			sem_post(&queueMutex);
			return -1;
		}

		/* initalize and alloc space for th's context */
		th->ucp = (ucontext_t *)malloc(sizeof(ucontext_t));
		if(th->ucp==NULL){
            printf("Fail to allocate space for th->ucp.\n");
            sem_post(&queueMutex);
            return -1;
        }
		getcontext(th->ucp); //initialize the context structure

		/* initalize and alloc the stack space for the thread's context */
		th->ucp->uc_stack.ss_sp = (void *)malloc(STACK_SIZE_BASE + offset);
		if(th->ucp->uc_stack.ss_sp == NULL){
	        printf("Fail to allocate space for th->ucp->uc_stack.ss_sp.\n");
	        sem_post(&queueMutex);
	        return -1;
        }

		printf("offset check %d\n", offset);
		th->ucp->uc_stack.ss_size = (STACK_SIZE_BASE + offset);

		makecontext(th->ucp, func, 0); //make the context for a thread running func

		/* set read of thread info values */
		th->next = NULL;
		th->runTime = 0;

		/* add the thread record into the queue */
		enQueue(th);
	}

	/* release control so waiting thread can execute */
	sem_post(&queueMutex);

	printf("uthread_create: exit\n");

	return 0;
}

void uthread_exit()
{
	int i;
	printf("uthread_exit: enter\n");

	/* Waiting so only one thread can enter the critical section */
	sem_wait(&queueMutex);

	/* get the thread id from system call*/
	pid_t tid = syscall(SYS_gettid);

	/* find index the thread in the array */
	for(i = 0; i < maxNumKThs; i++) {
		if(kthInfo[i].kthID == tid) {
			kthInfo[i].state = 0;
			break;
		}
	}

	/* Error if can't find thread */
	if(i == maxNumKThs){
		printf("Fail to locate kthInfo record for tid %d\n", tid);
		return;
	}

	if(kthInfo[i].th != NULL) {
		thread_info_destroy(kthInfo[i].th);
	}

	struct thread_info *th;

	/* exit process if queue is empty */
	if(head == NULL) {
		sem_post(&queueMutex);
		exit(0); //no more thread to run so terminate
	}

	/* pick up the thread record at the front of the queue */
	th = deQueue();

	/* swap ith kernal thread with dequeued thread */
	kthInfo[i].state = 1;
	kthInfo[i].runTime = th->runTime;
	gettimeofday(&kthInfo[i].startTime, NULL);
	kthInfo[i].th = th;

	/* release control so waiting thread can execute */
	sem_post(&queueMutex);

	printf("uthread_exit: exit\n");

	//set the context of the picked thread as the concurrent thread (i.e., run it!)
	setcontext(th->ucp);

}

int uthread_yield()
{
	int tid;

	printf("uthread_yield: enter\n");

	/* Waiting so only one thread can enter the critical section */
	sem_wait(&queueMutex);

	/* if there is nothing to dequeue then release control and return */
	if(head == NULL){
		sem_post(&queueMutex);

		printf("uthread_yield: exit 1\n");

		return 0;
	}

	/* get runtime of the head of the queue */
	unsigned long headRunTime=head->runTime;

	/* get thread id of current thread from system call */
	pid_t ttid = syscall(SYS_gettid);

	printf("tid=%d\n", ttid);

	/* find index of current thread */
	for(tid = 0; tid < maxNumKThs; tid++) {
		if(kthInfo[tid].kthID==ttid){
			printf("tid=%d\n", tid);
			break;
		}
	}

	/* Error if can't find thread */
	if(tid == maxNumKThs) {
		printf("Fail to find kthInfo record. Something is wrong!\n");
		return -1;
	}

	/* update the runtime of current thread */
	unsigned long runTime = kthInfo[tid].runTime + elapseTime(kthInfo[tid].startTime);

	/* 
		if runtime from queue is greater than current runtime don't yield. 
		release control and return. 
	*/
	if(headRunTime > runTime){
		sem_post(&queueMutex);

		printf("uthread_yield: exit 2\n");

		return 0;
	}

	/* construct queue thread object for current thread */

	/* create and allocate a thread_info object */
	struct thread_info *th; 

	if(kthInfo[tid].th == NULL) {
		/* initalize and alloc space for thread info object*/
		th = (struct thread_info *)malloc(sizeof(struct thread_info));
		if(th == NULL){
			printf("Fail to allocate space for th.\n");
			return -1;
		} 

		/* initalize and alloc space for th's context */
		th->ucp=(ucontext_t *)malloc(sizeof(ucontext_t));
		if(th->ucp==NULL){
			printf("Fail to allocate space for th->ucp.\n");
			return -1;
		}

		/* set object's runtime */
		th->runTime = runTime;
		kthInfo[tid].th = th;
	} else {
		th = kthInfo[tid].th;
		th->runTime = runTime;
	}

	/* enqueue the thread object */
	enQueue(th);

	/* swap head of queue into tidth kernal thread with dequeued thread */
	struct thread_info *th1;
	th1=deQueue();
	kthInfo[tid].runTime=th1->runTime;
	kthInfo[tid].th = th1;
	gettimeofday(&kthInfo[tid].startTime, NULL);

	/* release control so waiting thread can execute */
	sem_post(&queueMutex);

	/* swap th1 context with th context */
	swapcontext(th->ucp, th1->ucp);

	return 0;
}
