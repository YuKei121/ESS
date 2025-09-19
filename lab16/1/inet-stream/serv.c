#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define BACKLOG 5

int main() {
  int serverFd, clientFd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t clientLen = sizeof(client_addr);
  char buffer[BUFFER_SIZE];
  ssize_t bytesReceived;

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

  printf("Server (AF_INET, SOCK_STREAM) listening on port %d\n", PORT);

  while (1) {
    clientFd = accept(serverFd, (struct sockaddr*)&client_addr, &clientLen);
    if (clientFd == -1) {
      perror("accept failed");
      continue;
    }

    printf(
        "Client connected: %s:%d\n",
        inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port));

    bytesReceived = read(clientFd, buffer, BUFFER_SIZE - 1);
    if (bytesReceived == -1) {
      perror("read failed");
      close(clientFd);
      continue;
    }

    buffer[bytesReceived] = '\0';
    printf("Received from client: %s\n", buffer);

    const char* response = "hi!";
    if (write(clientFd, response, strlen(response)) == -1) {
      perror("write failed");
    }

    printf("Sent response: %s\n", response);
    close(clientFd);
  }

  close(serverFd);
  return 0;
}