#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define BACKLOG 5

void handleClient(int clientFd, struct sockaddr_in* client_addr) {
  char buffer[BUFFER_SIZE];
  char timeStr[50];
  time_t curTime;
  ssize_t bytesReceived;

  bytesReceived = recv(clientFd, buffer, BUFFER_SIZE - 1, 0);
  if (bytesReceived <= 0) {
    close(clientFd);
    exit(EXIT_FAILURE);
  }

  buffer[bytesReceived] = '\0';
  printf(
      "Process %d: Received from %s:%d: %s\n",
      getpid(),
      inet_ntoa(client_addr->sin_addr),
      ntohs(client_addr->sin_port),
      buffer);

  if (strcmp(buffer, "time") == 0) {
    curTime = time(NULL);
    ctime_r(&curTime, timeStr);

    send(clientFd, timeStr, strlen(timeStr), 0);
    printf("Process %d: Sent time to client\n", getpid());
  }

  close(clientFd);
  exit(EXIT_SUCCESS);
}

int main() {
  int serverFd, clientFd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t clientLen = sizeof(client_addr);
  pid_t pid;

  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd == -1) {
    perror("socket creation failed");
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

  printf("Multiproc server listening on port %d\n", PORT);

  while (1) {
    clientFd = accept(serverFd, (struct sockaddr*)&client_addr, &clientLen);
    if (clientFd == -1) {
      perror("accept failed");
      continue;
    }

    pid = fork();
    if (pid == 0) {
      close(serverFd);
      handleClient(clientFd, &client_addr);
    } else if (pid > 0) {
      close(clientFd);
      waitpid(-1, NULL, WNOHANG);
    } else {
      perror("fork failed");
      close(clientFd);
    }
  }

  close(serverFd);
  return 0;
}