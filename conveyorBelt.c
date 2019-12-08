#include <stdio.h>
#include "conveyor.h"

/*****************************************************/

/* conveyor belt and worker thread mutex lock */
pthread_mutex_t cLock = PTHREAD_MUTEX_INITIALIZER;

/* worker thread resume cond variable */
pthread_cond_t wResumeCond = PTHREAD_COND_INITIALIZER;

/* main() resume cond variable */
pthread_cond_t mResumeCond = PTHREAD_COND_INITIALIZER;

/*****************************************************/

int wCanResume = 0;

/* product/parts stats/counts and Locks */
int producedCount[X - A];
pthread_mutex_t producedCountLocks[X - A];

/*****************************************************/

/* conveyor belt thread */
void* convBeltThreadFn (void* arg) {

	int ret;
	unsigned int seed;
	struct timespec ts;
	struct timeval tp;

	while (1) {
		/* take conveyor belt, workers mutex lock */
		pthread_mutex_lock(&cLock);

		/* wait for all workers to be quiescent */
		while(finWorkers != WTC)
			pthread_cond_wait(&cCanResume, &cLock);

		/* increment run count */
		++run;

		/* run count exceeds max rotate count */
		if (run > ROTATECOUNT)
			goto quitconveyor;
		
		/* print conveyor banner */
		printf("\n################################\n");
		printf("# In the conveyor belt thread. #\n");
		printf("#  RUN  %3d  Shifting Conveyor #\n", run);
		printf("################################\n\n");

		/* if last conveyor slot is valid, get stats before overwritting */
		if (conveyor[CSIZE - 1].valid) {

			pthread_mutex_lock(&wastedCountLocks[conveyor[CSIZE - 1].type]);
			++wastedCount[conveyor[CSIZE - 1].type];
			pthread_mutex_unlock(&wastedCountLocks[conveyor[CSIZE - 1].type]);
		}

		/* shift conveyor slots */
		for (int i = CSIZE - 1; i >= 1; --i) { 

			if (conveyor[i - 1].valid) {
				//printf("\t%d --> %d\n", i - 1, i);
				pthread_mutex_destroy(&conveyor[i].sLock);
				conveyor[i] = conveyor[i - 1];
			}

			printf("----------");
		}

		printf("-----------\n");

		ret  = gettimeofday(&tp, NULL);

		/* init seed for randomizing type of item to be put in slot 0 */
		seed = (unsigned int)tp.tv_usec;
		srand(seed);
		/* init & insert new item at slot 0 */
		conveyor[0].type = rand()%3;
		conveyor[0].valid = 1;
		pthread_mutex_init(&conveyor[0].sLock, NULL);
		/* increment new type count */
		pthread_mutex_lock(&producedCountLocks[conveyor[0].type]);
		++producedCount[conveyor[0].type]; 
		pthread_mutex_unlock(&producedCountLocks[conveyor[0].type]);

		/* print conveyor state */
		for (int i = 0; i < CSIZE ; ++i) { 

			char ctype;

			if (!conveyor[i].valid)
				ctype = 'I';

			else
				ctype = getType(conveyor[i].type);

			printf("|    %c    ", ctype);
		}

		printf("|\n");

		for (int i = 0; i < CSIZE ; ++i) 
			printf("----------");

		printf("-\n\n");

		/* rei-init  worker thread counts */
		freeWorkers = WTC;
		finWorkers = 0;
		/* mark worker threads can resume now */
		wCanResume = 1;
		/* broadcast the same */
		pthread_cond_broadcast(&wResumeCond);

quitconveyor:

		/* release conveyor belt lock */
		pthread_mutex_unlock(&cLock);

		/* acquire exclusive access with main() */
		pthread_mutex_lock(&cQLock);
		/* if max runs have been completed signal main() */
		if (run > ROTATECOUNT)
			pthread_cond_signal(&mResumeCond);

		/* wait for quit signal from main() or timeout and continue */
		ret  = gettimeofday(&tp, NULL);
		ts.tv_sec = tp.tv_sec;
		ts.tv_nsec = tp.tv_usec*1000;
		ts.tv_sec += UNIT_TIME;

		while (!cThreadShouldQuit) {

			ret = pthread_cond_timedwait(&cQuitCond, &cQLock, &ts);
			/* 
			 * check first if quit condition is true thus making 
			 * sure this is not a spurious wake up
			 */
			if (cThreadShouldQuit) {

				printf("\n#### || Conveyor belt quitting || #####\n");

				for (int i = 0; i < CSIZE; ++i) {

					if (conveyor[i].valid) {

						pthread_mutex_lock(&wastedCountLocks[conveyor[i].type]);
						++wastedCount[conveyor[i].type];
						pthread_mutex_unlock(&wastedCountLocks[conveyor[i].type]);
					}
				}

				pthread_mutex_unlock(&cQLock);
				pthread_exit(NULL);
			}
			/* only then check if the thread timed out */
			if (ret == ETIMEDOUT)
				break;
		}

		pthread_mutex_unlock(&cQLock);
	}

	return NULL;
}
