#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BROADCAST_PORT 4444
#define BUFFER_SIZE 1024

int main() {
  int fd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t clientLen = sizeof(client_addr);
  char buffer[BUFFER_SIZE];
  ssize_t bytesReceived;
  int reuseAddr = 1;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd == -1) {
    perror("socket creation failed");
    return -1;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) ==
      -1) {
    perror("setsockopt SO_REUSEADDR failed");
    close(fd);
    return -1;
  }

  memset(&client_addr, 0, sizeof(client_addr));
  client_addr.sin_family = AF_INET;
  client_addr.sin_port = htons(BROADCAST_PORT);
  client_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
    perror("bind failed");
    close(fd);
    return -1;
  }

  printf("Broadcast client started. Listening on port %d\n", BROADCAST_PORT);

  while (1) {
    bytesReceived = recvfrom(
        fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &clientLen);
    if (bytesReceived == -1) {
      perror("recvfrom failed");
      continue;
    }

    buffer[bytesReceived] = '\0';
    printf(
        "Received from %s:%d: %s\n",
        inet_ntoa(server_addr.sin_addr),
        ntohs(server_addr.sin_port),
        buffer);
  }

  close(fd);
  return 0;
}