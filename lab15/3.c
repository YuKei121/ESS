#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  sigset_t set;
  int recvSig;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  sigprocmask(SIG_BLOCK, &set, NULL);

  for (int i = 0;; i++) {
    if (sigwait(&set, &recvSig) == 0) {
      if (recvSig == SIGUSR1) {
        printf("Iteration %d: Recieved SIGUSR1 signal from somewhere.\n", i);
      }
    }
  }
  return 0;
}