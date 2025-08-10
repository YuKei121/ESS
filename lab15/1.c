#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void signalHandler(int signal) {
  if (signal == SIGUSR1) {
    printf("Signal SIGUSR1 has arrived\n");
  }
}

int main() {
  struct sigaction customSA = {.sa_handler = signalHandler, .sa_flags = 0};
  sigemptyset(&customSA.sa_mask);
  sigaction(SIGUSR1, &customSA, NULL);
  while (1) {
    pause();
  }
  return 0;
}