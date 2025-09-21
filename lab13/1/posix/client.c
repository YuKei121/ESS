#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define QUEUE_NAME "/queue"
#define MAX_MSG_SIZE 100

int main() {
  mqd_t mq = mq_open(QUEUE_NAME, O_RDWR);
  if (mq == (mqd_t)-1) {
    perror("mq_open (client)");
    return -1;
  }

  printf("Connected to queue\n");

  char msg_in[MAX_MSG_SIZE];
  unsigned int prio;
  ssize_t bytes = mq_receive(mq, msg_in, MAX_MSG_SIZE, &prio);
  if (bytes == -1) {
    perror("mq_receive");
    return -1;
  }

  printf("Got the message: %s\n", msg_in);

  char msg_out[] = "Hello!";
  if (mq_send(mq, msg_out, strlen(msg_out) + 1, 1) == -1) {
    perror("mq_send");
    return -1;
  }

  printf("Sent a reply: %s\n", msg_out);

  mq_close(mq);

  return 0;
}