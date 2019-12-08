#ifndef _MYCONV_H_
#define _MYCONV_H_

#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>

/*****************************************************/

#define msg 			printf("%d: Assert Failed!\n", __LINE__)

#define assert(cond)	do {					\
							if (!(cond)) msg;	\
						} while (0)


#define DEBUG 	  		1
#define DCSIZE			3
#define DWTC 			2*DCSIZE
#define DSLOTS 			DCSIZE
#define DROTATECOUNT 	10
#define UNIT_TIME		1

/*****************************************************/

/* a conveyor belt slot */
typedef struct convBeltSlot {

  int type;
  int valid;
  pthread_mutex_t sLock;

} CBS;

extern CBS* conveyor;

/* worker threads info */
struct workerTInfo {

  int wTnum;
  pthread_t wThread;

};

/* various types of conveyor belt slot items */
enum iTypes {A=0, B, E, P, X};

extern int run;
extern int ROTATECOUNT;
extern int WTC;
extern int SKIPSLOTS;

extern pthread_cond_t mResumeCond;

extern int producedCount[];
extern int wastedCount[];
extern pthread_mutex_t producedCountLocks[];
extern pthread_mutex_t wastedCountLocks[];

extern int CSIZE;
extern int cThreadShouldQuit;

extern pthread_mutex_t cQLock;
extern pthread_mutex_t cLock;
extern pthread_cond_t cQuitCond;
extern pthread_cond_t cCanResume;

extern int WTC;
extern int finWorkers;
extern int freeWorkers;
extern int wCanResume;
extern int wThreadShouldQuit;
extern int* wTHands;

extern pthread_mutex_t wQLock;
extern pthread_cond_t wQuitCond;
extern pthread_cond_t wResumeCond;

void* workerThreadFn(void* );
void* convBeltThreadFn (void* );

/* returns type in char */
static inline char getType(int type) {

	char ctype;

	switch(type) {

		case A:
		case B:

			ctype = 'A' + type;
			break;

		case E:
			ctype = 'E';
			break;

		case P:
			ctype = 'P';
			break;

		default:
			break;
	} 

	return ctype;
}

#endif // _MYCONV_H_
