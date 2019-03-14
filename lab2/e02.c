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
int		tmax = 0;
int		flag = 0;
sem_t 	*s;


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
	pthread_t	th1;
	pthread_t	th2;

	s = (sem_t *) malloc(sizeof(sem_t));
	sem_init(s, 0, 0);

	if(pthread_create(&th1, NULL, th1_body, 0) != 0){
		printf("th1 creation failed\n");
		return 2;	
	}

	if(pthread_create(&th2, NULL, th2_body, 0) != 0){
		printf("th2 creation failed\n");
		return 3;	
	}

	pthread_join(th1,NULL);
	pthread_join(th2,NULL);	

	return 0;
}

void *th1_body(void *threadarg){
	// get tmax
	int tmax = (int *) threadarg;

	// sleep a random number of milliseconds `t` in a range 1 to 5
	int upper = 5;
	int lower = 1;
	unsigned long t = (rand() % (upper - lower + 1)) + lower; 
	struct timespec tt;
	ms2ts(&tt, t);
	nanosleep(&tt , NULL);  
	
	// print "waiting on semaphore after t milliseconds"
	printf("th1:\tWaiting on semaphore after %lu milliseconds\n", t);
	// wait on a semaphore `s` initialized to 0, no more than `tmax' milliseconds
	int res = wait_with_timeout(s,tmax);
	// print "wait returned normally" if `sem_post(s)` was performed by `th2`
	// within `tmax` milliseconds from the wait call or if the `sem_post` call is 
	// performed by `th2` before the `sem_wait` call performed by `th1`
	// otherwise in must print "wait on semaphore s returned for timeout"
	if(res == 1) {
		printf("th1:\tWait on semaphore s returned for timeout\n");
	} 
	else {
		printf("th1:\tWait returned normally\n");
	}

	// terminate
	pthread_exit(NULL);
}

void *th2_body(void *threadarg){
	// sleep a random number of milliseconds t in range 1000 to 10000
	int upper = 10000;
	int lower = 1000;
	unsigned long t = (rand() % (upper - lower + 1)) + lower; 
	struct timespec tt;
	ms2ts(&tt, t);
	nanosleep(&tt , NULL); 
 
	// print "performing signal on semaphore s after t milliseconds"
	printf("performing signal on semaphore s after %lu milliseconds\n", t);
	sem_post(s);
	
	// terminate
	pthread_exit(NULL);
}

void ms2ts(struct timespec *ts, unsigned long ms)
{
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000000;
}

int wait_with_timeout(sem_t *s, int tmax){
	// conversion from mseconds to useconds	
	int microseconds = tmax * 1000; 
	signal(SIGALRM, sig_alrm);
	ualarm(microseconds, 0);

	sem_wait(s);
	
	// check if the `sem_post` was delivered by 
	// `th2` or `sig_alrm`

	if(flag == 0){
		ualarm(0,0);
		return 0;
	}else{
		flag = 0;
		return 1;
	}
}

static void sig_alrm(int signo){
	ualarm(0, 0);
    sem_post(s);
    flag = 1;
}