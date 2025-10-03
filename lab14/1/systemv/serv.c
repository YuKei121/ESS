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
  int semServer = semget(SEM_SERVER_KEY, 1, IPC_CREAT | 0666);
  if (semServer == -1) {
    perror("semget server");
    return -1;
  }

  int semClient = semget(SEM_CLIENT_KEY, 1, IPC_CREAT | 0666);
  if (semClient == -1) {
    perror("semget client");
    return -1;
  }

  if (semctl(semServer, 0, SETVAL, 0) == -1) {
    perror("semctl SETVAL server");
    return -1;
  }
  if (semctl(semClient, 0, SETVAL, 0) == -1) {
    perror("semctl SETVAL client");
    return -1;
  }

  int shm = shmget(SHM_KEY, BUFFER_SIZE, IPC_CREAT | 0666);
  if (shm == -1) {
    perror("shmget");
    return -1;
  }

  char* shmPtr = (char*)shmat(shm, NULL, 0);
  if (shmPtr == (char*)-1) {
    perror("shmat");
    return -1;
  }

  strcpy(shmPtr, "Hi!");
  if (semop(semServer, &sigOp, 1) == -1) {
    perror("semop signal server");
    return -1;
  }

  printf("Sent hi to client, waiting for the reply\n");

  if (semop(semClient, &waitOp, 1) == -1) {
    perror("semop wait client");
    return -1;
  }

  printf("Client says: %s\n", shmPtr);

  if (shmdt(shmPtr) == -1) {
    perror("shmdt");
    return -1;
  }

  if (shmctl(shm, IPC_RMID, NULL) == -1) {
    perror("shmctl IPC_RMID");
    return -1;
  }

  if (semctl(semServer, 0, IPC_RMID) == -1) {
    perror("semctl IPC_RMID server");
    return -1;
  }
  if (semctl(semClient, 0, IPC_RMID) == -1) {
    perror("semctl IPC_RMID client");
    return -1;
  }
  return 0;
}