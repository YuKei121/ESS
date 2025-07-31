#include <pthread.h>
#include <stdio.h>

#define size 1000
int a = 0;
pthread_mutex_t mut;

void* func() {
  for (int i = 0; i < size; i++) {
    pthread_mutex_lock(&mut);
    ++a;
    pthread_mutex_unlock(&mut);
  }
}

int main() {
  pthread_t tid[size];
  int i;
  for (i = 0; i < size; i++) {
    pthread_create(&tid[i], NULL, func, NULL);
  }

  for (i = 0; i < size; i++) {
    pthread_join(tid[i], NULL);
  }
  printf("a = %d\n", a);
  return 0;
}