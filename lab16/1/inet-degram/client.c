#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int fd;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];
  ssize_t bytesReceived;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd == -1) {
    perror("socket creation failed");
    return -1;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
    perror("invalid address");
    close(fd);
    return -1;
  }

  const char* message = "hello!";
  if (sendto(
          fd,
          message,
          strlen(message),
          0,
          (struct sockaddr*)&server_addr,
          sizeof(server_addr)) == -1) {
    perror("sendto failed");
    close(fd);
    return -1;
  }

  printf("Sent to server: %s\n", message);

  bytesReceived = recvfrom(fd, buffer, BUFFER_SIZE, 0, NULL, NULL);
  if (bytesReceived == -1) {
    perror("recvfrom failed");
    close(fd);
    return -1;
  }

  buffer[bytesReceived] = '\0';
  printf("Received from server: %s\n", buffer);

  close(fd);
  return 0;
}