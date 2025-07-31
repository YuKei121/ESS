#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int shops[5];
int desires[3];
pthread_mutex_t m[5];

void* func(void* arg) {
  int i = *(int*)arg;
  free(arg);

  if (i < 3) {
    while (1) {
      int target = rand() % 5;
      pthread_mutex_lock(&m[target]);
      desires[i] -= shops[target];
      shops[target] = 0;
      printf("Customer №%d bought in shop %d\n", i, target);
      pthread_mutex_unlock(&m[target]);
      sleep(2);
      if (desires[i] <= 0)
        break;
    }
  } else {
    while (1) {
      int target = rand() % 5;
      pthread_mutex_lock(&m[target]);
      shops[target] += rand() % 550 + 500;
      pthread_mutex_unlock(&m[target]);
      printf("Producer №%d gave products in shop %d\n", i, target);
      sleep(1);
      if ((desires[0] <= 0) && (desires[1] <= 0) && (desires[2] <= 0))
        break;
    }
  }
}

int main() {
  srand(time(NULL));
  for (int i = 0; i < 5; i++) {
    shops[i] = rand() % 550 + 500;
    if (i < 3)
      desires[i] = rand() % 10000 + 9000;
  }
  pthread_t tid[4];
  for (int i = 0; i < 4; i++) {
    int* id = malloc(sizeof(int));
    *id = i;
    pthread_create(&tid[i], NULL, func, id);
  }
  for (int i = 0; i < 4; i++) {
    pthread_join(tid[i], NULL);
  }
  return 0;
}