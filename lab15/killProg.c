#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: %s <PID>\n", argv[0]);
    return 0;
  }
  char* endPtr;
  int pidNumb = strtol(argv[1], &endPtr, 10);
  if (*endPtr != '\0' || pidNumb <= 0) {
    printf("Wrong pid\n");
    return -1;
  }
  pid_t pid = (pid_t)pidNumb;

  // if (kill(pid, SIGINT) == -1) {
  // if (kill(pid, SIGKILL) == -1) {
  if (kill(pid, SIGUSR1) == -1) {
    perror("kill error");
    return -2;
  }
  printf("Signal sent successfully to process %d\n", pid);

  return 0;
}