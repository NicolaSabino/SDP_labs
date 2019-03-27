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

#define LOOP                  1000
#define MIN_SLEEP_PRODUCER    1
#define MAX_SLEEP_PRODUCER    10
#define SLEEP_CONSUMER        10
#define BUFF_SIZE			        16
#define PERCENTAGE				    80

typedef struct {
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    volatile unsigned n;
} sema_t;

typedef struct {
    int       				*buffer;
    size_t 					  head;
    size_t 				    tail;
    size_t 					  size;  //	final size of the buffer
    sema_t            full;  //	keep track of the number of full spots
    sema_t 					  empty; // 	keep track of the number of empty spots
    pthread_mutex_t 	mutex; //	mutual exclusion
    int 					    items;
} circular_buf_t;

/* GLOBAL VARIABLES */
circular_buf_t 		buffer;


/* PROTOTYPES */
int 				circular_buf_reset(circular_buf_t * cbuf);
int 				circular_buf_put(circular_buf_t * cbuf, int data);
int 				circular_buf_get(circular_buf_t * cbuf, int *data);
int 				circular_buf_empty(circular_buf_t cbuf);
int					circular_buf_full(circular_buf_t cbuf);
void 				*producer_body (void *threadarg);
void 				*consumer_body (void *threadarg);
void 				ms2ts(struct timespec *ts, unsigned long ms);




void sema_init(sema_t *semaphore, int n) {
    pthread_mutex_init(&semaphore->mutex,NULL);
    pthread_cond_init(&semaphore->condition,NULL);
    semaphore->n = n;
}

void sema_destroy(sema_t *semaphore) {
    pthread_cond_destroy(&semaphore->condition);
    pthread_mutex_destroy(&semaphore->mutex);
}

void sema_wait(sema_t * semaphore) {
    pthread_mutex_lock(&semaphore->mutex);  // begin cs
      while(semaphore->n == 0) {
          pthread_cond_wait(&semaphore->condition,&semaphore->mutex);  // wait on a condition
      }
      semaphore->n = semaphore->n - 1;  // decrement the counter
    pthread_mutex_unlock(&semaphore->mutex); // end cs
}

void sema_post(sema_t *semaphore) {
    pthread_mutex_lock(&semaphore->mutex);    // begin cs
      semaphore->n = semaphore->n + 1;        // increment the counter
    pthread_mutex_unlock(&semaphore->mutex);         // end cs
    pthread_cond_signal(&semaphore->condition);      // signal the condition
}

int  main(int argc, char **argv){


  srand(time(NULL));   // Initialization, should only be called once.

	/* initializzation of the NORMAL buffer*/
	buffer.size	= BUFF_SIZE;
	buffer.buffer = malloc(buffer.size);
	sema_init (&buffer.full, 0);
	sema_init (&buffer.empty, BUFF_SIZE);
	pthread_mutex_init (&buffer.mutex,NULL);
	circular_buf_reset (&buffer);


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
	sema_destroy(&buffer.empty);
	sema_destroy(&buffer.full);
	pthread_mutex_destroy(&buffer.mutex);
	pthread_mutex_destroy(&buffer.mutex);
  pthread_exit(NULL);

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

      int r = rand(); // Returns a pseudo-random integer between 0 and RAND_MAX

  		sema_wait(&buffer.empty);
  		pthread_mutex_lock(&buffer.mutex);
  			circular_buf_put(&buffer,r);
  			fprintf(stdout,"putting ->\t%d\tinto buffer\n",r);
  		pthread_mutex_unlock(&buffer.mutex);
  		sema_post(&buffer.full);
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
  		sema_wait(&buffer.full);
  		pthread_mutex_lock(&buffer.mutex);
          int data;
  				circular_buf_get(&buffer,&data);
  				fprintf(stdout,"getting <-\t%d\tfrom buffer\n",data);
  		pthread_mutex_unlock(&buffer.mutex);
  		sema_post(&buffer.empty); //increment the number of full slots
    }
  	pthread_exit(NULL);
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

	int circular_buf_put(circular_buf_t * cbuf, int data){
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

	int circular_buf_get(circular_buf_t * cbuf, int *data){
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
