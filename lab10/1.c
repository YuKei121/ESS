#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  pid_t pid = fork();
  if (pid == 0) {
    printf("Доч.процесс. PID: %d; PPID: %d\n", getpid(), getppid());
    exit(1);
  } else if (pid > 0) {
    printf("Род.процесс. PID: %d; PPID: %d\n", getpid(), getppid());
    wait(NULL);
  } else {
    perror("fork failed");
  }
  return 0;
}