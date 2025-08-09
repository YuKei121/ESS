#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  int pFd[2];
  pid_t pid;
  if (pipe(pFd) == -1) {
    perror("pipe error");
    return -1;
  }

  pid = fork();
  if (pid < 0) {
    perror("fork error");
    return -1;
  }

  if (pid == 0) {
    close(pFd[1]);
    char buf[100];
    int bytes = read(pFd[0], buf, sizeof(buf));
    if (bytes > 0) {
      printf("Child heard: %s\n", buf);
    }
    close(pFd[0]);
  } else {
    close(pFd[0]);
    printf("Parent says: Hi!\n");
    write(pFd[1], "Hi!", strlen("Hi!"));
    close(pFd[1]);
    wait(NULL);
  }
  return 0;
}