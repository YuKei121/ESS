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
  sem_t* semServer = sem_open(SEM_SERVER_NAME, O_CREAT, 0666, 0);
  sem_t* semClient = sem_open(SEM_CLIENT_NAME, O_CREAT, 0666, 0);

  if (semServer == SEM_FAILED || semClient == SEM_FAILED) {
    perror("sem_open");
    return -1;
  }

  int shmFd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  if (shmFd == -1) {
    perror("shm_open");
    return -1;
  }

  if (ftruncate(shmFd, BUFFER_SIZE) == -1) {
    perror("ftruncate");
    return -1;
  }

  char* shmPtr =
      mmap(0, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
  if (shmPtr == MAP_FAILED) {
    perror("mmap");
    return -1;
  }

  strcpy(shmPtr, "Hi!");
  printf("Sent hi to client\n");

  sem_post(semClient);
  sem_wait(semServer);
  printf("Client says: %s\n", shmPtr);
  munmap(shmPtr, BUFFER_SIZE);
  close(shmFd);

  shm_unlink(SHM_NAME);
  sem_close(semServer);
  sem_close(semClient);
  sem_unlink(SEM_SERVER_NAME);
  sem_unlink(SEM_CLIENT_NAME);
  return 0;
}