#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  int fd[2];
  char* istr1[16];
  char* istr2[16];
  int i, j, flag = 0;

  char* str = (char*)malloc(sizeof(char) * 128);
  char* command_1 = (char*)malloc(sizeof(char) * 128);
  char* command_2 = (char*)malloc(sizeof(char) * 128);

  fgets(str, 128, stdin);
  strtok(str, "\n");
  while (strcmp(str, "exit")) {
    for (i = 0; i < 16; i++) {
      istr1[i] = NULL;
    }
    i = 0;

    for (j = 0; j < 16; j++) {
      istr2[j] = NULL;
    }
    j = 0;

    flag = 0;

    istr1[i++] = strtok(str, " ");
    while (istr1[i - 1] != NULL) {
      istr1[i++] = strtok(NULL, " ");
      if (istr1[i - 1] != NULL && strcmp(istr1[i - 1], "|") == 0) {
        flag = 1;
        pipe(fd);
        istr1[i - 1] = NULL;
        break;
      }
      if (i >= 16)
        break;
    }
    if (flag == 1) {
      istr2[j++] = strtok(NULL, " ");
      while (istr2[j - 1] != NULL) {
        istr2[j++] = strtok(NULL, " ");
        if (j >= 16)
          break;
      }
    }

    sprintf(command_1, "/bin/%s", istr1[0]);
    if (flag == 1)
      sprintf(command_2, "/bin/%s", istr2[0]);

    if (fork()) {
      wait(NULL);

    } else {
      if (flag == 0) {
        execv(command_1, istr1);

      } else {
        if (fork()) {
          close(fd[0]);
          dup2(fd[1], 1);
          execv(command_1, istr1);
          wait(NULL);
        } else {
          close(fd[1]);
          dup2(fd[0], 0);
          execv(command_2, istr2);
        }
      }
    }

    fgets(str, 128, stdin);
    strtok(str, "\n");
  }
  free(str);
  return 0;
}