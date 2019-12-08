#include <stdio.h>
#include "conveyor.h"
/*****************************************************/

/* count of worker threads */
int freeWorkers = 0;

/* product/parts stats/counts and Locks */
int wastedCount[X - A];
pthread_mutex_t wastedCountLocks[X - A];

/*****************************************************/

/* WTC worker threads completed cond variable */
pthread_cond_t wComplCond = PTHREAD_COND_INITIALIZER;
/* conveyor resume cond variable */
pthread_cond_t cCanResume = PTHREAD_COND_INITIALIZER;

/*****************************************************/
static inline void workerUpdateStat() {
	/* increment product count */
	pthread_mutex_lock(&producedCountLocks[P]);
	++producedCount[P];
	pthread_mutex_unlock(&producedCountLocks[P]);

	/* decrement wasted part count A */
	pthread_mutex_lock(&wastedCountLocks[A]);
	--wastedCount[A];
	pthread_mutex_unlock(&wastedCountLocks[A]);

	/* decrement wasted part count B */
	pthread_mutex_lock(&wastedCountLocks[B]);
	--wastedCount[B];
	pthread_mutex_unlock(&wastedCountLocks[B]);
}

void* workerThreadFn(void* arg) {

	struct workerTInfo* wTInfo = arg;
	int tNum = wTInfo->wTnum;
	int slotNum = wTInfo->wTnum/2;
	int inHand = E;
	int slotType = E;
	int skipSlots = 0;
	int ret;
	struct timespec ts;
	struct timeval tp;

	while (1) {

		/* block till you get broadcast from conveyor thread */
		pthread_mutex_lock(&cLock);

		while (!wCanResume)
			pthread_cond_wait(&wResumeCond, &cLock);

		pthread_mutex_unlock(&cLock);

		/* get whats in hand */
		inHand = wTHands[tNum];

		/* if no slots to be skippped and conveyor slot is valid */
		if (!skipSlots && conveyor[slotNum].valid) {

			/* acquire aquire slot lock and inspect slot part type */
			pthread_mutex_lock(&conveyor[slotNum].sLock);

			// printf("Locked Slot Part       : %2c\t\t\t\t|\n", getType(conveyor[slotNum].type));

			/* get type of part in slot */
			slotType = conveyor[slotNum].type;
			/* if not holding a finished product in hand*/
			if (inHand != P) {

				/* if conveyor slot is not empty and is not having a finished product */
				if ( slotType != E && slotType != P) {

					int mate = tNum % 2 ? tNum - 1 : tNum + 1;
					/* 
					 * give up if hand is empty and mate needs this part
					 */
					if (inHand == E && wTHands[mate] == (slotType + 1) % 2)
						goto giveup4mate;
					/*
					 * else if hand is empty or hand has complimentary part
					 */
					if (inHand == E  || inHand == (slotType + 1) % 2) {
						/* 
						 * count this part as wasted till worker thread places this 
						 * back on the belt as a finished PRODUCT
						 */
						pthread_mutex_lock(&wastedCountLocks[slotType]);
						++wastedCount[slotType];
						pthread_mutex_unlock(&wastedCountLocks[slotType]);

						/* if nothing in hand, take part off conveyor slot */
						if (inHand == E)
							inHand = slotType; 

						else {
							/* convert hand to product */
							inHand = P; 
							/* start skip slots */
							skipSlots = SKIPSLOTS;
						}
						/* mark conveyor slot as empty */
						conveyor[slotNum].type = E;
					}
					/* else conveyor has same part as in hand, ignore it */
				} 
			} 
			/* else if holding a finished product and conveyor slot is empty */
			else if (slotType == E) {

				/* make sure hand has PRODUCT */
				assert(inHand==P);
				/* place finished product onto conveyor slot */
				conveyor[slotNum].type = inHand;

				/* update part/product stats */
				workerUpdateStat();

				/* mark hand as free */
				inHand = E;
			} 
			/* holding PRODUCT but no free slot, ignore it */
			// printf("Locked Slot Part       : %2c\t\t\t\t|\n", getType(conveyor[slotNum].type));
giveup4mate:
			/* update own hand*/
			wTHands[tNum] = inHand;
			/* release conveyor slot lock */
			pthread_mutex_unlock(&conveyor[slotNum].sLock);

		} 
		else if (skipSlots > 0)
				--skipSlots;
		//	else
		//	printf("Slot Type Invalid         : %2c\t\t\t\t|\n", 'I');

		pthread_mutex_lock(&cLock);

		/* print worker banner */
		printf("-----------------------------------------------------------\n");
		printf("| Worker Thread #        : %2d\t\t\t\t|\n", tNum);
		printf("| Slot #                 : %2d\t\t\t\t|\n", slotNum);
		printf("| In Hand Type           : %2c\t\t\t\t|\n", getType(inHand));
		printf("| Slots to skip          : %2d\t\t\t\t|\n", skipSlots);
		printf("-----------------------------------------------------------\n\n");

		--freeWorkers;
		/*
		 * if all worker threads got a shot
		 * then release all the waiting ones
		 */
		if (!freeWorkers)
			pthread_cond_broadcast(&wComplCond);

		/* wait for all worker threads to get a chance */
		else {

			while (freeWorkers)
				pthread_cond_wait(&wComplCond, &cLock);
		}

		/* make sure this/any worker doesn't resume before the conveyor belt resumes */
		wCanResume = 0;
		/* mark a worker as finished */
		++finWorkers;
		/* 
		 * print conveyor state after all workers have taken their hands off
		 * and signal conveyor belt to resume
		 */
		if (finWorkers == WTC) {

			for (int i = 0; i < CSIZE ; ++i) 
				printf("----------");

			printf("-\n");

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

			printf("-\n");

			pthread_cond_signal(&cCanResume);

		}

		pthread_mutex_unlock(&cLock);

		/* wait for quit broadcast signal from main() or timeout and continue */
		pthread_mutex_lock(&wQLock);

		ret = gettimeofday(&tp, NULL);
		ts.tv_sec = tp.tv_sec;
		ts.tv_nsec = tp.tv_usec*1000;
		ts.tv_sec += UNIT_TIME;

		while (!wThreadShouldQuit) {

			ret = pthread_cond_timedwait(&wQuitCond, &wQLock, &ts);
			/*
			 * check first if quit condition is true thus making 
			 * sure this is not a spurious wake up
			 */
			if (wThreadShouldQuit) {

				// update part/product stats before quitting
				if (!skipSlots && inHand == P)
					workerUpdateStat();

				printf("---- || Worker thread: %2d, with slotNum: %2d quitting || ----\n", tNum, slotNum);
				pthread_mutex_unlock(&wQLock);
				pthread_exit(NULL);
			}
			/* only then check if thread timed out */
			if (ret == ETIMEDOUT)
				break;
		}

	   	pthread_mutex_unlock(&wQLock);
	}

	return NULL;
}
