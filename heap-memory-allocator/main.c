// #include <sys/mmap.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  setvbuf(stdout, NULL, _IONBF, 0);

  printf("start\n");
  // fflush(stdout);

  int len = 30;
  
  int *arr = (int *)malloc(sizeof(int) * len);

  if (arr == NULL) {
    perror("Failed to get the memory");
    return 1;
  }

  printf("%p\n", (void *)arr);
  // fflush(stdout);


  free(arr);

  printf("end\n");
  // fflush(stdout);

  return 0;
}

