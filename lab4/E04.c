#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/times.h>
#include <limits.h>

/* PROTOTYPES */
float				**matrix_initializer();
float				**matrix_initializer_debug();
float   		*vector_initializer(int selector);
float   		*vector_initializer_debug(int selector);
float				float_rand( float min, float max);
void 				print_matrix(float **matrix);
void				print_vector(float *vector);
float				without_th();
float				with_th();

/* GLOBAL VARIABLES */
char 				*prog_name;
int  				k;
float				*v1;
float				*v2;
float 				*v;
float				**mat;
float				res1;
float				res2;
float 				sum;
int					counter;
pthread_mutex_t		dec_counter;
pthread_mutex_t		final_command;





int main(int argc, char **argv){

	prog_name 	= argv[0];
	if (argc != 2 ) {
		printf("Usage: %s <k>\n", prog_name);
		return -1;
    }

    k   = atoi(argv[1]);
    v	= vector_initializer_debug(0);
	v1  = vector_initializer_debug(1);
	v2  = vector_initializer_debug(1);
	mat	= matrix_initializer_debug();


	//print_matrix(mat);
	//print_vector(v1);
	//print_vector(v2);

    res1 = without_th();
    res2 = with_th();


    printf("Result whithout threads:\t%f\n",res1);
    printf("Result whith threads:\t%f\n",res2);

	return 0;

}

float without_th(){
	sum = 0;

	float *v = (float *) malloc(k*sizeof(float));

	for(int i=0; i<k;i++){			// for each row of mat
		for(int j=0; j<k; j++){		// for each col of v2
			v[i] += mat[i][j] * v2[j];	//cumulate the products
		}
	}

	for(int i=0; i<k; i++){
		sum += v1[i]*v[i];
	}

	return sum;

}

void *thread_body(){

	/* row selection */
	static int row = -1;
	row++;



	/* perform the product of the i-th row vector of mat and v2 */
	for(int i = 0; i<k ; i++){
		v[row] += mat[row][i] * v2[i];
	}

	pthread_mutex_lock(&dec_counter);
		counter--;
	pthread_mutex_unlock(&dec_counter);

	pthread_mutex_trylock(&final_command);
	if (counter == 0){
       /* last command */
       for(int i=0; i<k; i++){
				 sum += v1[i]*v[i];
			 }
			printf("the last thread is %lx\n", pthread_self());
    }
	pthread_mutex_unlock(&final_command);

	pthread_exit(NULL);

	}

float with_th(){

	sum = 0;
	counter = k;

	/* creation of K threads*/
	pthread_t threads[k];
	pthread_mutex_init(&dec_counter, NULL);
	pthread_mutex_init(&final_command, NULL);

	/* run all trhreads */
	for(int i=0;i<k;i++){
		pthread_create( &threads[i], NULL, thread_body, NULL);
	}

	/* wait all threads */
	for(int i=0; i<k; i++){
		pthread_join(threads[i], NULL);

	}

	return sum;

	}

float *vector_initializer(int selector){
		float *pointer;
		pointer = (float *)malloc(k*sizeof(float));

		for(int i=0;i<k;i++){
			if(selector){
				pointer[i] = float_rand(-0.5,0.5);
			}else{
				pointer[i] = 0;
			}
		}

		return pointer;

	}

float *vector_initializer_debug(int selector){
		float *pointer;
		pointer = (float *)malloc(k*sizeof(float));

		for(int i=0;i<k;i++){
			if(selector){
				pointer[i] = i;
			}else{
				pointer[i] = 0;
			}
		}

		return pointer;

	}

float **matrix_initializer(){

	float **pointer;

	pointer = malloc(k*sizeof(float *));

	for(int i=0; i<k;i++){
		 pointer[i] = malloc( k * sizeof(float));
	}

	for(int i=0;i<k;i++){
		for(int j=0;j<k;j++){
			pointer[i][j] = float_rand(-0.5,0.5);
		}
	}
	return pointer;
}

float **matrix_initializer_debug(){

	float **pointer;

	pointer = malloc(k*sizeof(float *));

	for(int i=0; i<k;i++){
		 pointer[i] = malloc( k * sizeof(float));
	}

	for(int i=0;i<k;i++){
		for(int j=0;j<k;j++){
			pointer[i][j] = i;
		}
	}
	return pointer;
}

float float_rand( float min, float max ){
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

void print_matrix(float **matrix){
		printf("\n");
		for(int i=0; i<k; i++){
			for(int j=0;j<k;j++){
				printf("%f\t",matrix[i][j]);
			}
			printf("\n");
		}
}

void print_vector(float *vector){
		printf("\n");
		for(int i=0; i<k; i++){
			printf("%f\n",vector[i]);
		}
}
