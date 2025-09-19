#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/local_stream_socket"
#define BUFFER_SIZE 1024

int main() {
  int serverFd, clientFd;
  struct sockaddr_un server_addr, client_addr;
  socklen_t clientLen = sizeof(client_addr);
  char buffer[BUFFER_SIZE];
  ssize_t bytesReceived;

  serverFd = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (serverFd == -1) {
    perror("socket creation failed");
    return -1;
  }

  remove(SOCKET_PATH);

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sun_family = AF_LOCAL;
  strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

  if (bind(serverFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("bind failed");
    close(serverFd);
    return -1;
  }

  if (listen(serverFd, 5) == -1) {
    perror("listen failed");
    close(serverFd);
    return -1;
  }

  printf("Server (AF_LOCAL, SOCK_STREAM) listening on %s\n", SOCKET_PATH);

  while (1) {
    clientFd = accept(serverFd, (struct sockaddr*)&client_addr, &clientLen);
    if (clientFd == -1) {
      perror("accept failed");
      continue;
    }

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
  remove(SOCKET_PATH);
  return 0;
}