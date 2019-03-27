#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/wait.h>

#define LOOP 					1000
#define MIN_SLEEP_PRODUCER 		1
#define MAX_SLEEP_PRODUCER 		10
#define SLEEP_CONSUMER			10
#define BUFF_SIZE				16
#define PERCENTAGE				80				

typedef struct {
    long long 				*buffer;
    size_t 					head;
    size_t 					tail;
    size_t 					size;  //	final size of the buffer 
    sem_t 					full;  //	keep track of the number of full spots
    sem_t 					empty; // 	keep track of the number of empty spots 
    pthread_mutex_t 		mutex; //	mutual exclusion
    int 					items;
} circular_buf_t;

/* GLOBAL VARIABLES */
circular_buf_t 		urgent;
circular_buf_t 		normal;

	
/* PROTOTYPES */
int 				circular_buf_reset(circular_buf_t * cbuf);
int 				circular_buf_put(circular_buf_t * cbuf, long long data);
int 				circular_buf_get(circular_buf_t * cbuf, long long *data);
int 				circular_buf_empty(circular_buf_t cbuf);
int					circular_buf_full(circular_buf_t cbuf);
long long 			current_timestamp();
void 				*producer_body (void *threadarg);
void 				*consumer_body (void *threadarg);
int 				choose_buffer();
void 				ms2ts(struct timespec *ts, unsigned long ms);


int  main(int argc, char **argv){
	
	
	/* initializzation the URGENT buffer*/
	urgent.size	= BUFF_SIZE;
	urgent.buffer = malloc(urgent.size);
	sem_init (&urgent.full, 0, 0);
	sem_init (&urgent.empty, 0, BUFF_SIZE);
	pthread_mutex_init (&urgent.mutex,NULL);
	circular_buf_reset (&urgent);
	
	/* initializzation of the NORMAL buffer*/
	normal.size	= BUFF_SIZE;
	normal.buffer = malloc(urgent.size);
	sem_init (&normal.full, 0, 0);
	sem_init (&normal.empty, 0, BUFF_SIZE);
	pthread_mutex_init (&normal.mutex,NULL);
	circular_buf_reset (&normal);
	
	
	/* creation of two threads*/
	pthread_t	producer;
	pthread_t	consumer;
	
	if (pthread_create(&producer, NULL, producer_body, NULL) != 0){
        printf("\nProducer creation failed\n");
        return 1;
    } 
	  
    if (pthread_create(&consumer, NULL, consumer_body,NULL) != 0){
        printf("\nConsumer creation failed\n");
        return 1;
    }
        
    //idle
    pthread_join(producer, NULL);	// wait for producer
    pthread_join(consumer, NULL);	// wait for consumer
	
	// release resources
	sem_destroy(&urgent.empty);
	sem_destroy(&urgent.full);
	sem_destroy(&normal.empty);
	sem_destroy(&normal.full);
	pthread_mutex_destroy(&urgent.mutex);
	pthread_mutex_destroy(&normal.mutex);
	
    pthread_exit(NULL);
}

/**
 * Provided method
 */
long long current_timestamp() {
	struct timeval te;
	gettimeofday(&te, NULL);
	long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
	return milliseconds;
}


	/**
	 * Producer
	 */
	void *producer_body(void *threadarg){
		
		/* producer loop */
		for(int loop_producer = LOOP; loop_producer > 0;loop_producer--){
			
			// sleep a random number of milliseconds t in range 1 to 10
			int upper = MAX_SLEEP_PRODUCER * 1000;
			int lower = MIN_SLEEP_PRODUCER * 1000;
			unsigned long t = (rand() % (upper - lower + 1)) + lower; 
			struct timespec tt;
			ms2ts(&tt, t);
			nanosleep(&tt , NULL); 
			
			long long ms;
			ms = current_timestamp();

			if(choose_buffer()){
				/* URGENT */
				
				/* If there are no empty slots, wait */
				sem_wait(&urgent.empty);
				pthread_mutex_lock(&urgent.mutex);
					circular_buf_put(&urgent,ms);
					fprintf(stdout,"putting\t%llu\tin URGENT\n",ms);
				pthread_mutex_unlock(&urgent.mutex);
				sem_post(&urgent.full);
				
			}else{
				/* NORMAL */
				
				sem_wait(&normal.empty);
				pthread_mutex_lock(&normal.mutex);
					circular_buf_put(&normal,ms);
					fprintf(stdout,"putting\t%llu\tin NORMAL\n",ms);
				pthread_mutex_unlock(&normal.mutex);
				sem_post(&normal.full);

			}
		}

		pthread_exit(NULL);
	}

	/**
	 *  Consumer
	 */
	void *consumer_body(void *threadarg){
		
		/* consumer loop */
		for(int loop_consumer = LOOP; loop_consumer > 0; loop_consumer--){
			
			// sleep 10 milliseconds
			unsigned long t = SLEEP_CONSUMER * 1000; 
			struct timespec tt;
			ms2ts(&tt, t);
			nanosleep(&tt , NULL); 
			
			long long data;
				

				// try to acquire the urgent buffer
				if (sem_trywait(&urgent.full) == 0) {// if is not empty
					pthread_mutex_lock(&urgent.mutex); // begin cs
							circular_buf_get(&urgent,&data);
							fprintf(stdout,"getting\t%llu\tfrom URGENT\n",data);
					pthread_mutex_unlock(&urgent.mutex); // end cs
					sem_post(&urgent.empty); //increment the number of full slots
				} else {
					sem_wait(&normal.full);
					pthread_mutex_lock(&normal.mutex);
							circular_buf_get(&normal,&data);
							fprintf(stdout,"getting\t%llu\tfrom NORMAL\n",data);
					pthread_mutex_unlock(&normal.mutex);
					sem_post(&normal.empty); //increment the number of full slots
				}
		}
		

		pthread_exit(NULL);
	}


	 int choose_buffer(){
		 int random_number = rand() % 100;	// pick a random number between 1 and 100
		 if(random_number > PERCENTAGE)	  	// if the number is greather than the PERCENTAGE threshold 
			 return 1;							// select the urgent buffer
		 else
			 return 0;							// otherwise select the normal one
	}

	int circular_buf_reset(circular_buf_t * cbuf){
		int r = -1;

		if(cbuf)
		{
		    cbuf->head = 0;
		    cbuf->tail = 0;
		    cbuf->items = 0;
		    r = 0;
		}

		return r;
	}

	int circular_buf_empty(circular_buf_t cbuf){
		// We define empty as head == tail
		if(cbuf.head == cbuf.tail)
			return 1;
		else
			return 0;
			
	}

	int circular_buf_full(circular_buf_t cbuf){
		// We determine "full" case by head being one position behind the tail
		// Note that this means we are wasting one space in the buffer!
		// Instead, you could have an "empty" flag and determine buffer full that way
		if (((cbuf.head + 1) % cbuf.size) == cbuf.tail)
			return 1;
		else
			return 0;
	}

	int circular_buf_put(circular_buf_t * cbuf, long long data){
		int r = -1;

		if(cbuf)
		{
		    cbuf->buffer[cbuf->head] = data;
		    cbuf->head = (cbuf->head + 1) % cbuf->size;

		    if(cbuf->head == cbuf->tail)
		    {
		        cbuf->tail = (cbuf->tail + 1) % cbuf->size;
		    }
		cbuf->items++;
		    r = 1;
		}

		return r;
	}

	int circular_buf_get(circular_buf_t * cbuf,long long *data){
		int r = -1;
		if(cbuf && data && !circular_buf_empty(*cbuf)){
		    *data = cbuf->buffer[cbuf->tail];
		    cbuf->tail = (cbuf->tail + 1) % cbuf->size;
		    cbuf->items--;
		    r = 0;
		}

		return r;
	}

	void ms2ts(struct timespec *ts, unsigned long ms){
    	ts->tv_sec = ms / 1000;
    	ts->tv_nsec = (ms % 1000) * 1000000;
	}