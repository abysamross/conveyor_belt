#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "conveyor.h"

/*****************************************************/

/* conveyor belt size */
int CSIZE = DCSIZE; 
/* Max conveyor runs */
int  ROTATECOUNT = DROTATECOUNT;
/* run count */
int run = 0;

/* conveyor belt */
CBS* conveyor;

/* conveyor belt thread info */
pthread_t convBeltThread;

/*****************************************************/

/* worker thread count */
int WTC = DWTC;
/* finshed worker thread count */
int finWorkers = DWTC;
/* slots to be skipped */
int SKIPSLOTS = DSLOTS; 
/* worker thread hands */
int* wTHands;

/*****************************************************/

/* various thread unicast/broadcast flags */
int wThreadShouldQuit = 0;
int cThreadShouldQuit = 0; 

/*****************************************************/

/* worker thread quit cond variable and mutex */
pthread_cond_t wQuitCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t wQLock = PTHREAD_MUTEX_INITIALIZER;
/* conveyor belt thread quit cond variable and mutex */
pthread_cond_t cQuitCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t cQLock = PTHREAD_MUTEX_INITIALIZER;

/*****************************************************/
int main(int argc, char* argv[]) {

	void *res;

	if (argc == 3) {

		CSIZE 		= atoi(argv[1]);
		ROTATECOUNT = atoi(argv[2]);
		WTC 		= 2*CSIZE;
		SKIPSLOTS   = CSIZE;
		finWorkers = WTC;

	} else if (argc > 1) {

		printf("Usage! %s [number of slots [=3]] [number of runs [=10]]\n", argv[0]);
		return 1;
	}

	/* allocate memory and init conveyor belt */
	conveyor = malloc(CSIZE*sizeof(CBS));
	for (int i = 0; i < CSIZE; ++i)
		memset(&conveyor[i], 0, sizeof(CBS)); 

	/* allocate memory and init worker thread infos */
	struct workerTInfo wTInfo[WTC];
	memset(wTInfo, 0, sizeof(wTInfo));

	/* allocate memory and init worker thread hands */
	wTHands = malloc(WTC*sizeof(int));
	for (int i = 0; i < WTC; ++i)
		wTHands[i] = E;

	/* init stat locks */
	for (int i = 0; i < X - A; ++i) {

		pthread_mutex_init(&wastedCountLocks[i], NULL);
		pthread_mutex_init(&producedCountLocks[i], NULL);
	}

	printf("Conveyor belt\n");
	/* Put your implementation and any tests here. */

	/* create conveyor belt thread */
	if (pthread_create(&convBeltThread, NULL, convBeltThreadFn, NULL)) {

		perror("Error creating conveyor belt thread");
		return 1;
	}
	 
	/* create worker threads */
	for (int i = 0; i < WTC; ++i) {

		wTInfo[i].wTnum = i;

		if (pthread_create(&wTInfo[i].wThread, NULL, workerThreadFn, &wTInfo[i])) {

			perror("Error creating worker thread");
			fprintf(stderr, "thread num: %d\n", i);
	  }
	}

	/* block main() till # runs equal ROTATECOUNT */
	pthread_mutex_lock(&cQLock);

	while (run <= ROTATECOUNT)
		pthread_cond_wait(&mResumeCond, &cQLock);

	pthread_mutex_unlock(&cQLock);

	/* broadcast workers to quit */
	pthread_mutex_lock(&wQLock);
	wThreadShouldQuit = 1;
	pthread_cond_broadcast(&wQuitCond);
	pthread_mutex_unlock(&wQLock);

	/* wait for all worker threads to join */
	for (int i = 0; i < WTC; ++i) {
	  
	  if (pthread_join(wTInfo[i].wThread, &res)) { 

		  perror("Error waiting for worker thread");
		  fprintf(stderr, "thread num: %d\n", i);
	  }

	}

	/* signal conveyor belt to quit */
	pthread_mutex_lock(&cQLock);
	cThreadShouldQuit = 1;
	pthread_cond_signal(&cQuitCond);
	pthread_mutex_unlock(&cQLock);

	/* wait for conveyor belt thread to join */
	if (pthread_join(convBeltThread, &res))
		perror("Error waiting for conveyer belt thread");

	/* print stats */
	printf("\n\n");
	
	printf("################################\n\n");

	int diffA = producedCount[A] - wastedCount[A];
	int diffB = producedCount[B] - wastedCount[B];

	assert(diffA == diffB);
	assert(diffA == producedCount[P]);
	assert(diffB == producedCount[P]);

	printf("\n\n");

	for (int i = 0; i < X - A; ++i) {

		switch(i) {

			case A:
			case B:

				printf("Part `%c' Produced: %d\n", ('A'+i), producedCount[i]);
				printf("Part `%c' Wasted: %d\n\n", ('A'+i), wastedCount[i]);
				break;

			case E:

				printf("Null slots generated: %d\n\n", producedCount[i]);
				break;
			
			case P:

				printf("Finished Product Count: %d\n\n", producedCount[i]);
				break;

			default:
				break;
		}
		/* free stat mutexex */
		pthread_mutex_destroy(&wastedCountLocks[i]);
		pthread_mutex_destroy(&producedCountLocks[i]);
	}

	free(wTHands);
	free(conveyor);
	return 0;
}
