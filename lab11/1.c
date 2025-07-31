#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* func(void* i) {
  int arg = *(int*)i;
  printf("Thread %d\n", arg);
  free(i);
}

int main() {
  pthread_t tid[5];
  int i;
  for (i = 0; i < 5; i++) {
    int* id = malloc(sizeof(int));
    *id = i;
    pthread_create(&tid[i], NULL, func, id);
  }

  for (i = 0; i < 5; i++) {
    pthread_join(tid[i], NULL);
  }
  return 0;
}