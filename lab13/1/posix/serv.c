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
  struct mq_attr attr = {
      .mq_flags = 0, .mq_maxmsg = 10, .mq_msgsize = MAX_MSG_SIZE};

  mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0666, &attr);
  if (mq == (mqd_t)-1) {
    perror("mq_open (server)");
    return -1;
  }

  char msg_out[] = "Hi!";
  if (mq_send(mq, msg_out, strlen(msg_out) + 1, 0) == -1) {
    perror("mq_send");
    return -1;
  }

  printf("Sent a message %s\n", msg_out);

  // for testing purposes only: better to create another queue
  sleep(10);

  char msg_in[MAX_MSG_SIZE];
  unsigned int prio;
  ssize_t bytes = mq_receive(mq, msg_in, MAX_MSG_SIZE, &prio);
  if (bytes == -1) {
    perror("mq_receive");
    return -1;
  }

  printf("Got a reply %s\n", msg_in);

  mq_close(mq);
  mq_unlink(QUEUE_NAME);

  return 0;
}