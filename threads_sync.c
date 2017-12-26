#define _XOPEN_SOURCE 600
#include <linux/unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
//#define PTHREAD_SYNC 1
#define __NR_print_tasks_darien_morrison 342

extern long int syscall (long int __sysno, ...) __THROW;

int SharedVariable = 0;

#ifdef PTHREAD_SYNC
pthread_barrier_t barr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/* Entry point for the thread. This is the "main" function for each thread */ 
void * SimpleThread(void* which)
{
	int num, val;

	for(num = 0; num < 20; num++)
	{
		if(random() > RAND_MAX / 2)
			usleep(10);

		#ifdef PTHREAD_SYNC
		pthread_mutex_lock(&mutex);		
		#endif

		val = SharedVariable;

		printf("*** Thread %d sees value %d\n", (int)which, val);

		SharedVariable = val + 1;

		#ifdef PTHREAD_SYNC
		pthread_mutex_unlock(&mutex);
		#endif
	}

	#ifdef PTHREAD_SYNC
	int rc = pthread_barrier_wait(&barr);
	if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
	{
		printf("Could not wait on barrier.\n");
		exit(-1);
	}
	#endif

	val = SharedVariable;

	printf("Thread %d sees final value %d\n", (int)which, val);
}


/* Function to check whether command line parameter is a valid integer */
bool isValid(char integers[])
{
	int i = 0;
	
	// For all the characters in the "number"
	for(; i < strlen(integers); i++)
	{
		// If the character is not a digit, return false
		if(!isdigit(integers[i]))
			return false;
	}

	return true;
}



int main(int argc, char ** argv)
{
	if(argc <= 1)
		fprintf(stderr, "Cannot run program without specifying thread count.\n");

	// Check if user enters an integer or garbage
	if(isValid(argv[1]))
	{
		// Size of thread array
		int input = atoi(argv[1]);

		int i = 0;		
		int j = 0;	

		// Thread array 
		pthread_t thr[input];

		#ifdef PTHREAD_SYNC
		// Barrier initialization
		if(pthread_barrier_init(&barr, NULL, input))
		{
			printf("Could not create barrier.\n");
			return -1;
		}
		#endif

		// Create threads up to the number user entered
		for(; i < input; i++)
		{
			// Check if thread creation failed
			if(pthread_create(&thr[i], NULL, &SimpleThread, (void *) i))
			{
				printf("Could not create thread\n");
				return -1;
			}		
		}

		// System call
		syscall(__NR_print_tasks_darien_morrison);

		// Join all the threads
		for(; j < input; j++)
		{
			// Check if thread joining failed
			if(pthread_join(thr[j], NULL))
			{
				printf("Could not join thread\n");
				return -1;
			}	
		}

		// Clean up the mutex
		pthread_mutex_destroy(&mutex);
	}
	// User entered an invalid parameter
	else
	{
		fprintf(stderr, "ERROR: Input parameter must be a positive integer.\n");
		exit(1);
	}

	return 0;
}
