#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int fd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t clientLen = sizeof(client_addr);
  char buffer[BUFFER_SIZE];
  ssize_t bytesReceived;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd == -1) {
    perror("socket creation failed");
    return -1;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("bind failed");
    close(fd);
    return -1;
  }

  printf("Server (AF_INET, SOCK_DGRAM) listening on port %d\n", PORT);

  while (1) {
    bytesReceived = recvfrom(
        fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &clientLen);
    if (bytesReceived == -1) {
      perror("recvfrom failed");
      continue;
    }

    buffer[bytesReceived] = '\0';
    printf(
        "Received from client %s:%d: %s\n",
        inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port),
        buffer);

    const char* responseMessage = "hi!";
    if (sendto(
            fd,
            responseMessage,
            strlen(responseMessage),
            0,
            (struct sockaddr*)&client_addr,
            clientLen) == -1) {
      perror("sendto failed");
    }

    printf("Sent response: %s\n", responseMessage);
  }

  close(fd);
  return 0;
}