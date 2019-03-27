#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>

#define	FILE1 "fv1.b"
#define	FILE2 "fv2.b"
#define FILE3 "fv3.b"

// Define binary tree node
typedef struct arguments {
	int			file_items;
	char*		filename;
	pthread_t 	thread;
} Infos;


/* PROTOTYPES */
 void 	fill_vector(int *vector, int dimension, int lower_bound, int upper_bound);
 void		generate_file(char *filename, int *vector, int dimension);
 void 	*client_body (void *threadarg);
 int		random_number (int min, int max);
 void 	print_file(char * filename, int dimension);

/* GLOBAL VARIABLES */
 int 				g;
 int 				requests_counter;
 int 				tasks;
 sem_t 				*sem1,*sem2;
 pthread_mutex_t 	*lock;


int main(int argc, char **argv){

	if(argc != 3){
		printf("wrong parameters\n");
		return 0;
	}

	int n1 = atoi(argv[1]);
	int n2 = atoi(argv[2]);

	int v1[n1];
	int v2[n2];

	fill_vector(v1,n1,10,100);
	fill_vector(v2,n2,21,101);

	generate_file(FILE1,v1, n1);
	generate_file(FILE2,v2, n2);

	// print all the numers stored in
	// FILE1 and FILE2
	// print_file(FILE1,n1);
	// print_file(FILE2,n2);


	sem1 = (sem_t *) malloc(sizeof(sem_t));
	sem2 = (sem_t *) malloc(sizeof(sem_t));
	lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));


	// semaphores initializations
	sem_init(sem1, 0, 0);
	sem_init(sem2, 0, 0);

	pthread_mutex_init(lock, NULL);

	Infos client1;
	Infos client2;

	client1.file_items 	= n1;
	client1.filename	= FILE1;

    if (pthread_create(&client1.thread, NULL, client_body, &client1) != 0){
        printf("\n thread cration 1 failed\n");
        return 1;
    }

	client2.file_items	= n2;
	client2.filename	= FILE2;

    if (pthread_create(&client2.thread, NULL, client_body, &client2) != 0){
        printf("\n thread creation 2 failed\n");
        return 1;
    }


	tasks = n1 + n2-1;
	requests_counter = 0;

	// server body
	do{
		sem_wait(sem1);
		g = g*3;
		sem_post(sem2);
	}while(tasks != 0);


	sleep(0.5);
	printf("Total requests:\t %d\n",requests_counter);


	sem_destroy(sem1);
	sem_destroy(sem2);

	pthread_mutex_destroy(lock);

	return 0;
	//pthread_exit(NULL);



}

/**
 * random_number
 * ------------
 * return a pseudo integer number between min an max
 */
int random_number(int min, int max){
	return rand() % (max-min) + min;
}

/**
 * fill_vector
 * ------------
 * Function that fills each vector with random numbers
 */
void fill_vector(int *vector, int dimension, int lower_bound, int upper_bound){
	for(int i=0;i<dimension;i++){
		vector[i] = random_number(lower_bound, upper_bound);
	}
}

/**
 * print_file
 * ------------
 * Utility function that print each number stored in a binary file
 * [#] num
 */
void print_file(char *filename, int dimension){
	FILE *f;
	f = fopen(filename,"rb");
	printf("%s\n",filename);
	int value;
	for( int i=0; i< dimension;i++){
		fread(&value,sizeof(int),1,f);
		printf("[%d] %d\n",i,value);
    }
    fclose(f);
}

/**
 * generate_file
 * ------------
 * Function that stores a given vector in a binary file
 */
void generate_file(char *filename,int *vector, int dimension){
	FILE *fpw;
	if ((fpw = fopen (filename, "wb")) == NULL){
		fprintf(stderr," error open %s\n", filename);
	}

	fwrite(vector,sizeof(int),dimension, fpw);
	fclose(fpw);

}

/**
 * client_body
 * ------------
 * Body of a client thread.
 * Each thread is associated with an Infos data structure
 * containing the filename and the number of items
 * of the file.
 */
void *client_body(void *threadarg){

	pthread_t 	identifier = pthread_self();
	int 		value;
	Infos 		*info = (Infos *) threadarg;
	FILE *f;
	f = fopen(info->filename,"rb");

    for(int i=0; i< info->file_items;i++){
		fread(&value,sizeof(int),1,f);

		//critical section
		pthread_mutex_lock(lock);
			g = value;
			sem_post(sem1);
			sem_wait(sem2);
			printf("[Transaction #%d][Thread %li] g = %d\n",requests_counter, identifier, g);
			requests_counter++;
			tasks--;
		pthread_mutex_unlock(lock);
		//end of critical section
    }



    fclose(f);
	pthread_exit(NULL);
}
