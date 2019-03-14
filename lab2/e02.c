#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <time.h> // nanosleep()

/* PROTOTYPES */
void	*th1_body(void *threadarg);
void 	*th2_body(void *threadarg);
void 	ms2ts(struct timespec *ts, unsigned long ms);
int		wait_with_timeout(sem_t *s, int tmax);
static void sig_alrm(int signo);

/* GLOBAL VARIABLES */
int	tmax;
sem_t 				*s;

int main(int argc, char **argv){
	
	// Use current time as seed for random generator 
	srand(time(0));

	// semaphore initializations
	sem_init(s, 0, 0);

	if(argc != 2){
		printf("wrong parameters");
		return 1;
	}

	// tmax is passed as an argument of the command line
	int tmax = atoi(argv[1]);	

	return 0;
}

void *th1_body(void *threadarg){
	// sleep a random number of milliseconds `t` in a range 1 to 5
	int upper = 5;
	int lower = 1;
	unsigned long t = (rand() % (upper - lower + 1)) + lower; 
	struct timespec tt;
	ms2ts(&tt, t);
	nanosleep(&tr , NULL);  
	
	// print "waiting on semaphore after t milliseconds"
	printf("waiting on semaphore after %lu milliseconds\n", t);
	// wait on a semaphore `s` initialized to 0, no more than `tmax' milliseconds

	// print "wait returned normally" if `sem_post(s)` was performed by `th2`
	// within `tmax` milliseconds from the wait call or if the `sem_post` call is 
	// performed by `th2` before the `sem_wait` call performed by `th1`

	// otherwise in must print "wait on semaphore s returned for timeout"

	// terminate
	exit(0);
}

void *th2_body(void *threadarg){
	// sleep a random number of milliseconds t in range 1000 to 10000
	int upper = 10000;
	int lower = 1000;
	unsigned long t = (rand() % (upper - lower + 1)) + lower; 
	struct timespec tt;
	ms2ts(&tt, t);
	nanosleep(&tr , NULL); 
 
	// print "performing signal on semaphore s after t milliseconds"
	printf("performing signal on semaphore s after %lu milliseconds\n", t);
	
// terminate
}

void ms2ts(struct timespec *ts, unsigned long ms)
{
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000000;
}

int wait_with_timeout(sem_t *s, int tmax){
	// conversion from mseconds to useconds	
	int microseconds = tmax * 1000; 
	ualarm(microseconds,0);	
}

static void sig_alrm(int signo){
	return;
}