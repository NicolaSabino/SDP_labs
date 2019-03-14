#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void filler(int *array, int size, int lb,int ub, int flag);
void print(int *array,int size);
void selectionSort(int *array, int size);
void txtFilePrinter(char *filename, int *array, int size);
void binFilePrinter(char *filename, int *array, int size);

int main(int argc, char *argv[]){
  // check arguments
  if(argc != 2+1){
    printf("wrong parameters\n");
    return 0;
  }

  /* Intializes random number generator */
  time_t t;
  srand((unsigned) time(&t));

  // takes from the command line two integer numbers n1 , n2
  int n1 = atoi(argv[1]);
	int n2 = atoi(argv[2]);
  // allocates two vectors v1 and v2 , of dimensions n1 and n2
  int v1[n1];
  int v2[n2];

  // fills v1 with n1 random even integer numbers between 10-100
  filler(v1,n1,10,100,1);
  // fills v2 with n2 random odd integer numbers between 21-101
  filler(v2,n2,21,101,0);
  #if defined DEBUG
      printf("before sorting\n");
       print(v1,n1);
       print(v2,n2);
  #endif
  // sort v1 and v2 (increasing values)
  selectionSort(v1,n1);
  selectionSort(v2,n2);
  #if defined DEBUG
      printf("after sorting\n");
      print(v1,n1);
      print(v2,n2);
  #endif
  // save the content of vectors v1 and v2 in two text files fv1.txt and fv2.txt
  txtFilePrinter("fv1.txt",v1,n1);
  txtFilePrinter("fv2.txt",v2,n2);
  // save the content of vectors v1 and v2 in two binary files fv1.b and fv2.b
  binFilePrinter("fv1.b",v1,n1);
  binFilePrinter("fv2.b",v2,n2);
  return 0;
}

void filler(int *array, int size, int lb,int ub, int flag){
  for (size_t i = 0; i < size; i++) {
    array[i] = (rand() % (ub - lb + 1)) + lb;
    if(flag){// even
      array[i]+=(array[i] % 2 == 0 ? 0 : 1);
    }else{ // odd
      array[i]+=(array[i] % 2 != 0 ? 0 : 1);
    }
  }
}

void selectionSort(int *array, int size){
  for (size_t i = 0; i < size; i++) {
    for (size_t j = i+1; j < size; j++) {
      if (array[j] < array[i]){
        int tmp = array[j];
        array[j] = array[i];
        array[i] = tmp;
      }
    }
  }
}

void print(int *array, int size) {
  for (size_t i = 0; i < size; i++) {
    printf("%d\t",array[i]);
  }
  printf("\n");
}
void txtFilePrinter(char *filename, int *array, int size){
  FILE *fpw; // file pointer
  if ((fpw = fopen (filename, "w")) == NULL){ // w for write, b for binary
    fprintf(stderr," error open %s\n", filename);
    return;
  }
  for (size_t i = 0; i < size; i++) {
    fprintf(fpw,"%d\t",array[i]);
  }
  fclose(fpw);
}

void binFilePrinter(char *filename, int *array, int size){
  FILE *fpw; // file pointer
  if ((fpw = fopen (filename, "wb")) == NULL){ // w for write, b for binary
    fprintf(stderr," error open %s\n", filename);
    return;
  }
  fwrite(array,sizeof(int),size, fpw); // write sizeof(buffer) bytes from bufferfclose(fpw)
  fclose(fpw);
}
