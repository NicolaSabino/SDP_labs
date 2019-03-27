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

typedef struct infos {
	int low;
	int high;
} infos_t;

/* PROTOTYPES */

int		mapping_procedure(char* file_name);
void	quicksort (int left, int right);
void	print_vector(int dim);
void	swap(int *a, int *b);
int 	partition(int low, int high);
void 	quickSort(int low, int high);
void 	*thread_body(void *threadarg);

/* GLOBAL VARIABLES */
char *prog_name;
char *file_name;
int	 *numbers;
int  size;
int  items;


int main(int argc, char **argv){

	prog_name 	= argv[0];
	if (argc != 3 ) {
		printf("Usage: %s <file_name.bin><size>\n", prog_name);
		return -1;
    }

	file_name = argv[1];
	size	  = atoi(argv[2]);

	items = mapping_procedure(file_name);
	if(items == -1){
		fprintf(stderr,"An error occours");
		return -1;
	}

	//print_vector(items);

	infos_t infos;
	infos.low 	= 0;
	infos.high	= items-1;

	pthread_t firstThread;
	pthread_create ( &firstThread, NULL, thread_body, &infos);
	pthread_join(firstThread, NULL);


	//printf("\n");
	//print_vector(items);

	return 0;

}

void  *thread_body (void *threadarg){
	infos_t	*old_inf;
	old_inf	= (infos_t *) threadarg;

	if (old_inf->low < old_inf->high)
    {
			/*
			 * pi is partitioning index, arr[p] is now
			 * at right place
			 */
			int pi = partition(old_inf->low, old_inf->high);

		if((old_inf->high - old_inf->low)< size){
			//RECURSIVE QUICK SORT

			// Separately sort elements before
			// partition and after partition
			quickSort( old_inf->low		, pi - 1);
			quickSort( pi + 1	, old_inf->high);

		}else{
			// MULTI THREAD QUICK SORT

			pthread_t partA, partB;
			infos_t infoA,infoB;

			infoA.low 	= old_inf->low;
			infoA.high	= pi-1;

			infoB.low	= pi+1;
			infoB.high	= old_inf->high;

			pthread_create ( &partA, NULL, thread_body, &infoA);
			pthread_create ( &partB, NULL, thread_body, &infoB);
			pthread_join(partA, NULL);
			pthread_join(partB,NULL);

		}


    }

	pthread_exit(NULL);
}

int mapping_procedure(char* file_name){
	long int len,len2;
	int fd, pg;
  	struct stat stat_buf;

	if ((fd = open (file_name, O_RDWR)) == -1){
		perror ("open");
		return -1;
	}

	if (fstat (fd, &stat_buf) == -1){
		perror ("fstat");
		return -1;
	}
	len = stat_buf.st_size;
	len2 = len;
	pg = getpagesize ();
	//printf ("Page size %d\n", pg);
	//printf ("File size in byte %ld\n", len);
	len = len + pg - (len % pg);
	//printf ("File size rounded to page size %ld\n", len);
	numbers = mmap ((caddr_t) 0, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	return len2/sizeof(int);
}

/*
 * This function takes last element as pivot, places
 * the pivot element at its correct position in sorted
 * array, and places all smaller (smaller than pivot)
 * to left of pivot and all greater elements to right
 * of pivot
 */
int partition (int low, int high){
    int pivot 	= numbers[high];    // pivot
    int i 		= (low - 1);  		// Index of smaller element

    for (int j = low; j <= high- 1; j++){

		// If current element is smaller than or
        // equal to pivot
        if (numbers[j] <= pivot){
            i++;    // increment index of smaller element
            swap(&numbers[i], &numbers[j]);
        }
    }
    swap(&numbers[i + 1], &numbers[high]);
    return (i + 1);
}

/*
 * The main function that implements QuickSort
 * arr[] --> Array to be sorted,
 * low   --> Starting index,
 * high  --> Ending index
 */
void quickSort(/*int arr[],*/ int low, int high){
    if (low < high)
    {
        /*
		 * pi is partitioning index, arr[p] is now
         * at right place
         */
        int pi = partition(low, high);

        // Separately sort elements before
        // partition and after partition
        quickSort( low		, pi - 1);
        quickSort( pi + 1	, high);
    }
}



void swap(int* a, int* b){
    int t = *a;
    *a = *b;
    *b = t;
}

void print_vector(int dim){
	for( int i=0; i<dim;i++){
		fprintf(stdout,"%p\t%d\n",&numbers[i],numbers[i]);
	}
}
