#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_MSG_SIZE 256
#define SHM_KEY 0x12345
#define SEM_KEY 0x12346
#define HISTORY_SIZE 100

struct message {
  char text[MAX_MSG_SIZE];
  pid_t pid;
};

struct sharedData {
  int messageCount;
  struct message messages[HISTORY_SIZE];
};

int shm;
struct sharedData* sharedMemory;
int sem;

void cleanup() {
  shmdt(sharedMemory);
  shmctl(shm, IPC_RMID, NULL);
  semctl(sem, 0, IPC_RMID);
  exit(0);
}

void signalHandler(int sig) {
  cleanup();
}

int main() {
  signal(SIGINT, signalHandler);

  shm = shmget(SHM_KEY, sizeof(struct sharedData), IPC_CREAT | 0666);
  if (shm < 0) {
    perror("shmget");
    return -1;
  }

  sharedMemory = shmat(shm, NULL, 0);
  if (sharedMemory == (void*)-1) {
    perror("shmat");
    return -1;
  }

  // Initialize shared memory
  memset(sharedMemory, 0, sizeof(struct sharedData));

  // Create semaphore
  sem = semget(SEM_KEY, 1, IPC_CREAT | 0666);
  if (sem < 0) {
    perror("semget");
    cleanup();
    return -1;
  }

  semctl(sem, 0, SETVAL, 1);

  printf("Server started\n");

  while (1) {
    pause();
  }

  cleanup();
  return 0;
}