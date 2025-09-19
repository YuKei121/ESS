#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/local_dgram_socket"
#define BUFFER_SIZE 1024

int main() {
  int fd;
  struct sockaddr_un server_addr, client_addr;
  socklen_t clientLen = sizeof(client_addr);
  char buffer[BUFFER_SIZE];
  ssize_t bytesReceived;

  fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (fd == -1) {
    perror("socket creation failed");
    return -1;
  }

  remove(SOCKET_PATH);

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sun_family = AF_LOCAL;
  strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

  if (bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("bind failed");
    close(fd);
    return -1;
  }

  printf("Server (AF_LOCAL, SOCK_DGRAM) listening on %s\n", SOCKET_PATH);

  while (1) {
    bytesReceived = recvfrom(
        fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &clientLen);
    if (bytesReceived == -1) {
      perror("recvfrom failed");
      continue;
    }

    buffer[bytesReceived] = '\0';
    printf("Received from client: %s\n", buffer);

    const char* response = "hi!";
    if (sendto(
            fd,
            response,
            strlen(response),
            0,
            (struct sockaddr*)&client_addr,
            clientLen) == -1) {
      perror("sendto failed");
    }

    printf("Sent response: %s\n", response);
  }

  close(fd);
  remove(SOCKET_PATH);
  return 0;
}