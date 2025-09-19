#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define POOL_SIZE 5
#define BACKLOG 5

void handleClient(int clientFd, struct sockaddr_in* client_addr) {
  char buffer[BUFFER_SIZE];
  char timeStr[50];
  time_t curTime;
  ssize_t bytesReceived;

  bytesReceived = recv(clientFd, buffer, BUFFER_SIZE - 1, 0);
  if (bytesReceived <= 0) {
    close(clientFd);
    return;
  }

  buffer[bytesReceived] = '\0';
  printf(
      "Worker %d: Received from %s:%d: %s\n",
      getpid(),
      inet_ntoa(client_addr->sin_addr),
      ntohs(client_addr->sin_port),
      buffer);

  if (strcmp(buffer, "time") == 0) {
    curTime = time(NULL);
    ctime_r(&curTime, timeStr);
    send(clientFd, timeStr, strlen(timeStr), 0);
    printf("Worker %d: Sent time to client\n", getpid());
  }

  close(clientFd);
}

void sigchld_handler(int sig) {
  signal(SIGCHLD, sigchld_handler);
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

int main() {
  int serverFd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t clientLen = sizeof(client_addr);
  pid_t pids[POOL_SIZE];
  int i;

  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, sigchld_handler);

  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd == -1) {
    perror("socket creation failed");
    return -1;
  }

  int reuse = 1;
  if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) ==
      -1) {
    perror("setsockopt failed");
    close(serverFd);
    return -1;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(serverFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("bind failed");
    close(serverFd);
    return -1;
  }

  if (listen(serverFd, BACKLOG) == -1) {
    perror("listen failed");
    close(serverFd);
    return -1;
  }

  printf("Preforked pool server listening on port %d\n", PORT);
  printf("Created pool of %d workers\n", POOL_SIZE);

  for (i = 0; i < POOL_SIZE; i++) {
    pids[i] = fork();
    if (pids[i] == 0) {
      printf("Worker %d started\n", getpid());
      while (1) {
        int clientFd =
            accept(serverFd, (struct sockaddr*)&client_addr, &clientLen);
        if (clientFd == -1) {
          perror("accept failed");
          continue;
        }
        printf(
            "Worker %d: accepted connection from %s:%d\n",
            getpid(),
            inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port));
        handleClient(clientFd, &client_addr);
      }
    } else if (pids[i] < 0) {
      perror("fork failed");
    }
  }

  printf("Main process %d on standby\n", getpid());
  while (1) {
    sleep(1);
    for (i = 0; i < POOL_SIZE; i++) {
      if (kill(pids[i], 0) == -1) {
        printf("Worker %d died, restart\n", pids[i]);
        pids[i] = fork();
        if (pids[i] == 0) {
          printf("Restarted worker %d started\n", getpid());
          while (1) {
            int clientFd =
                accept(serverFd, (struct sockaddr*)&client_addr, &clientLen);
            if (clientFd == -1) {
              perror("accept failed");
              continue;
            }
            handleClient(clientFd, &client_addr);
          }
        }
      }
    }
  }

  close(serverFd);
  return 0;
}