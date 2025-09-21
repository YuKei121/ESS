#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

struct msgbuf {
  long mtype;
  char mtext[100];
};

int main() {
  key_t key = ftok("/tmp", 'A');
  if (key == -1) {
    perror("ftok");
    return -1;
  }

  int msqid = msgget(key, 0666);
  if (msqid == -1) {
    perror("msgget");
    return -1;
  }

  printf("Connected to queue\n");

  struct msgbuf msg_in;
  if (msgrcv(msqid, &msg_in, sizeof(msg_in.mtext), 1, 0) == -1) {
    perror("msgrcv");
    return -1;
  }

  printf("Got a message: %s\n", msg_in.mtext);

  struct msgbuf msg_out = {2, "Hello!"};
  if (msgsnd(msqid, &msg_out, strlen(msg_out.mtext) + 1, 0) == -1) {
    perror("msgsnd");
    return -1;
  }

  printf("Sent a reply: %s\n", msg_out.mtext);

  return 0;
}