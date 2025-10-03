#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHM_NAME "/testShm"
#define SEM_SERVER_NAME "/semServer"
#define SEM_CLIENT_NAME "/semClient"
#define BUFFER_SIZE 256

int main() {
  sem_t* semServer = sem_open(SEM_SERVER_NAME, 0);
  sem_t* semClient = sem_open(SEM_CLIENT_NAME, 0);

  if (semServer == SEM_FAILED || semClient == SEM_FAILED) {
    perror("sem_open");
    return -1;
  }

  sem_wait(semClient);

  int shmFd = shm_open(SHM_NAME, O_RDWR, 0666);
  if (shmFd == -1) {
    perror("shm_open");
    return -1;
  }

  char* shmPtr =
      mmap(0, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
  if (shmPtr == MAP_FAILED) {
    perror("mmap");
    return -1;
  }

  printf("Server says: %s\n", shmPtr);

  strcpy(shmPtr, "Hello!");
  sem_post(semServer);
  munmap(shmPtr, BUFFER_SIZE);
  close(shmFd);

  sem_close(semServer);
  sem_close(semClient);

  return 0;
}