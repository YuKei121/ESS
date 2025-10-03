#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 0x1234

// server -> client
#define SEM_SERVER_KEY 0x5678

// client -> server
#define SEM_CLIENT_KEY 0x5679

#define BUFFER_SIZE 256

struct sembuf waitOp = {0, -1, 0};
struct sembuf sigOp = {0, 1, 0};

int main() {
  int semServer = semget(SEM_SERVER_KEY, 1, 0666);
  if (semServer == -1) {
    perror("semget server");
    return -1;
  }

  int serClient = semget(SEM_CLIENT_KEY, 1, 0666);
  if (serClient == -1) {
    perror("semget client");
    return -1;
  }

  printf("Waiting for server\n");
  if (semop(semServer, &waitOp, 1) == -1) {
    perror("semop wait server");
    return -1;
  }

  int shm = shmget(SHM_KEY, BUFFER_SIZE, 0666);
  if (shm == -1) {
    perror("shmget");
    return -1;
  }

  char* shmPtr = (char*)shmat(shm, NULL, 0);
  if (shmPtr == (char*)-1) {
    perror("shmat");
    return -1;
  }

  printf("Server says: %s\n", shmPtr);

  strcpy(shmPtr, "Hello!");
  if (semop(serClient, &sigOp, 1) == -1) {
    perror("semop signal client");
    return -1;
  }

  if (shmdt(shmPtr) == -1) {
    perror("shmdt");
    return -1;
  }
  return 0;
}