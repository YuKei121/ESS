#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigprocmask(SIG_BLOCK, &set, NULL);

  while (1) {
    pause();
  }
  return 0;
}