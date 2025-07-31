#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  pid_t pid = fork();
  if (pid == 0) {
    printf("Доч.процесс 1.1. PID: %d; PPID: %d\n", getpid(), getppid());
    pid_t pid3 = fork();
    if (pid3 == 0) {
      printf("Доч.процесс 1.1.1. PID: %d; PPID: %d\n", getpid(), getppid());
      exit(1);
    } else if (pid3 > 0) {
      pid_t pid4 = fork();
      if (pid4 == 0) {
        printf("Доч.процесс 1.1.2. PID: %d; PPID: %d\n", getpid(), getppid());
      } else if (pid4 > 0) {
        wait(NULL);
      } else {
        perror("fork errror");
      }
      wait(NULL);
    } else {
      perror("fork error");
    }
    exit(1);
  } else if (pid > 0) {
    printf("Род.процесс 1. PID: %d; PPID: %d\n", getpid(), getppid());
    pid_t pid2 = fork();
    if (pid2 == 0) {
      printf("Доч.процесс 1.2. PID: %d; PPID: %d\n", getpid(), getppid());
      pid_t pid5 = fork();
      if (pid5 == 0) {
        printf("Доч.процесс 1.2.1. PID: %d; PPID: %d\n", getpid(), getppid());
        exit(1);
      } else if (pid > 0) {
        wait(NULL);
      } else {
        perror("fork error");
      }
      exit(1);
    } else if (pid2 > 0) {
      wait(NULL);
    } else {
      perror("fork error");
    }
    wait(NULL);
  } else {
    perror("fork failed");
  }
  return 0;
}