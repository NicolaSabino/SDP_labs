#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

void filler(int *array, int size, int lb,int ub, int flag);
void print(int *array,int size);
void selectionSort(int *array, int size);
void txtFilePrinter(char *filename, int *array, int size);
void binFilePrinter(char *filename, int *array, int size);
void child1Routine(int n1);
void child2Routine(int n2);

int main(int argc, char *argv[]){
  // check arguments
  if(argc != 2+1){
    printf("wrong parameters\n");
    return 0;
  }

  // takes from the command line two integer numbers n1 , n2
  int n1 = atoi(argv[1]);
	int n2 = atoi(argv[2]);


  /* Intializes random number generator */
  time_t t;
  srand((unsigned) time(&t));

  pid_t child1; // child 1 process identifier
  pid_t child2; // child 2 process identifier
  int child_process = 0;
  int status;

  child1 = fork();  // fork the first child
  if(child1 == 0) child1Routine(n1);  // if is the first child lunch the routine
  else child_process++;
  child2 = fork();  // fork the second child
  if(child2 == 0) child2Routine(n2);  // if is the second child lunch the routine
  else child_process++;
  while (child_process != 0) {
    wait(&status); // wait for a process
    printf("Proces %d ends \n", status);
    child_process--;
  }
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
void child1Routine(int n1){
  // define an array v1 of dimension n1
  int v1[n1];
  // fills v1 with n1 random even integer numbers between 10-100
  filler(v1,n1,10,100,1);
  // sort v1 (increasing values)
  selectionSort(v1,n1);
  // save the content of vector v1 in fv1.txt
  txtFilePrinter("fv1.txt",v1,n1);
  // save the content of vector v1 in fv1.b
  binFilePrinter("fv1.b",v1,n1);
  // return its identity
  exit(1);
}
void child2Routine(int n2){
  // define an array v2 of dimension n2
  int v2[n2];
  // fills v2 with n2 random odd integer numbers between 21-101
  filler(v2,n2,21,101,0);
  // sort v2 (increasing values)
  selectionSort(v2,n2);
  // save the content of vector v2 in fv2.txt
  txtFilePrinter("fv2.txt",v2,n2);
  // save the content of vector v2 in fv2.b
  binFilePrinter("fv1.b",v2,n2);
  // return its identity
  exit(2);
}
