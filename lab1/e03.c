#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>

/* GLOBAL VARIABLES */
char  next; // managed by input thread
char  this; // managed by processing thread
char  last; // out<put thread
int   flag = 1;
FILE  *fp;

/* prototypes */
void 	*thread_input (void *threadarg);
void 	*thread_processing (void *threadarg);
void 	*thread_output (void *threadarg);


int main(int argc, char *argv[]){

  if(argc != 2){
    printf("Wrong paramenters\n");
    return 1;
  }

  pthread_t  input, processing, output;

  if((fp = fopen(argv[1], "r+")) == NULL){
    printf("No such file\n");
    exit(1);
  }

  this = NULL;
  next = NULL;
  last = NULL;
  while(flag){
      last = this;
      this = next;
      (void) pthread_create(&input, NULL, thread_input, NULL);
      (void) pthread_create(&processing, NULL, thread_processing, NULL);
      (void) pthread_create(&output, NULL, thread_output, NULL);
      (void) pthread_join(input, NULL);
      (void) pthread_join(processing, NULL);
      (void) pthread_join(output, NULL);
  }

  return 0;
}

void *thread_input(void *threadarg){
  if(fscanf(fp,"%c",&next)!=1){
    next = NULL;
    flag = 0;
  }

	pthread_exit(NULL);
}

void *thread_processing(void *threadarg){
  if (this >= 'a' && this <= 'z' && this != NULL) {
         this = this - 32;
  }
	pthread_exit(NULL);
}

void *thread_output(void *threadarg){
  printf("%c",last);
	pthread_exit(NULL);
}
