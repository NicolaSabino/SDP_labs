#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

/*typedef struct node {
	int 	      	identifier;
	int		        depth;
	int         	path_lenght;
	long	      	path[1000];
	pthread_t   	l_thread;
	pthread_t   	r_thread;
} Node;*/

/* GLOBAL VARIABLES */
char next; // managed by input thread
char this; // managed by processing thread
char last; // output thread

/* prototypes */
void 	*thread_input (void *threadarg);
void 	*thread_processing (void *threadarg);
void 	*thread_output (void *threadarg);

int main(int argc, char *argv[]){

  if(argc != 2){
    printf("Wrong paramenters\n");
    return 1;
  }

  FILE *fp;
  if((fp = fopen(argv[1], "r+")) == NULL){
    printf("No such file\n");
    exit(1);
  }
  char character;
  while (fscanf(fp,"%c",&character) == 1) {
    pthread_t  input,processing,output;
    (void) pthread_create(&input, NULL, thread_input, &character);
    if(this != NULL)  (void) pthread_create(&processing, NULL, thread_processing, &this);
    if(this != NULL)  (void) pthread_create(&output, NULL, thread_output, &last);
    (void) pthread_join(input, NULL);
    (void) pthread_join(processing, NULL);
    (void) pthread_join(output, NULL);
  }

  return 0;
}
