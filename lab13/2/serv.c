#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 256
#define SERVER_KEY 0x12345

struct message {
  long mtype;
  char mtext[MAX_MSG_SIZE];
  pid_t pid;
};

struct clientInfo {
  pid_t pid;
  char nickname[50];
};

struct clientInfo clients[MAX_CLIENTS];
int clientsQuantity = 0;
int msqid;

void cleanup() {
  msgctl(msqid, IPC_RMID, NULL);
  exit(0);
}

void signalHandler(int sig) {
  cleanup();
}

void broadcastMessage(struct message* msg, pid_t sender_pid) {
  char senderName[50] = "God knows who";

  for (int i = 0; i < clientsQuantity; i++) {
    if (clients[i].pid == sender_pid) {
      strncpy(senderName, clients[i].nickname, sizeof(senderName) - 1);
      senderName[sizeof(senderName) - 1] = '\0';
      break;
    }
  }

  char formattedMessage[MAX_MSG_SIZE];
  int written = snprintf(
      formattedMessage,
      sizeof(formattedMessage),
      "[%s]: %s",
      senderName,
      msg->mtext);

  if (written >= sizeof(formattedMessage)) {
    formattedMessage[sizeof(formattedMessage) - 1] = '\0';
  }

  struct message broadcastingMessage = {1, "", 0};
  strncpy(
      broadcastingMessage.mtext,
      formattedMessage,
      sizeof(broadcastingMessage.mtext) - 1);
  broadcastingMessage.mtext[sizeof(broadcastingMessage.mtext) - 1] = '\0';

  for (int i = 0; i < clientsQuantity; i++) {
    if (clients[i].pid != sender_pid) {
      broadcastingMessage.mtype = clients[i].pid;
      msgsnd(
          msqid,
          &broadcastingMessage,
          sizeof(struct message) - sizeof(long),
          0);
    }
  }
}

void checkMessage(struct message* msg) {
  if (msg->mtype == 1) {
    if (clientsQuantity < MAX_CLIENTS) {
      clients[clientsQuantity].pid = msg->pid;
      strncpy(
          clients[clientsQuantity].nickname,
          msg->mtext,
          sizeof(clients[clientsQuantity].nickname) - 1);
      clientsQuantity++;
      printf("Client registered: %s (PID: %d)\n", msg->mtext, msg->pid);
    }
  } else if (msg->mtype == 2) {
    broadcastMessage(msg, msg->pid);
  } else if (msg->mtype == 3) {
    for (int i = 0; i < clientsQuantity; i++) {
      if (clients[i].pid == msg->pid) {
        printf("Client exited: %s (PID: %d)\n", clients[i].nickname, msg->pid);
        memmove(
            &clients[i],
            &clients[i + 1],
            (clientsQuantity - i - 1) * sizeof(struct clientInfo));
        clientsQuantity--;
        break;
      }
    }
  }
}

int main() {
  signal(SIGINT, signalHandler);

  if ((msqid = msgget(SERVER_KEY, IPC_CREAT | 0666)) < 0) {
    perror("msgget");
    return -1;
  }

  printf("Server started. MSQID: %d\n", msqid);

  while (1) {
    struct message msg;
    if (msgrcv(msqid, &msg, sizeof(struct message) - sizeof(long), 0, 0) < 0) {
      perror("msgrcv");
      continue;
    }
    checkMessage(&msg);
  }

  return 0;
}