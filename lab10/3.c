#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARGCOUNT 16

int main() {
  char* buf = (char*)malloc(sizeof(char) * 128);
  char* command = (char*)malloc(sizeof(char) * 128);
  char* args[ARGCOUNT];
  int i;
  fgets(buf, 128, stdin);
  strtok(buf, "\n");
  while (strcmp(buf, "exit")) {
    for (i = 0; i < 16; i++)
      args[i] = NULL;
    i = 0;
    args[i++] = strtok(buf, " ");

    while (args[i - 1] != NULL) {
      args[i++] = strtok(NULL, " ");
      if (i >= ARGCOUNT)
        break;
    }

    sprintf(command, "/bin/%s", args[0]);
    pid_t idf = fork();
    if (idf < 0) {
      perror("fork error");
      exit(1);
    }
    if (idf == 0) {
      execv(command, args);
    } else {
      wait(NULL);
    }
    fgets(buf, 128, stdin);
    strtok(buf, "\n");
  }
  return 0;
}