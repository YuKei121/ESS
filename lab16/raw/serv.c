#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void modifyData(char* str) {
  if (strlen(str) > 0) {
    int index = rand() % strlen(str);
    str[index] = (str[index] + 1) % 256;
  }
}

int main() {
  int sockFd;
  struct sockaddr_in servAddr, clientAddr;
  socklen_t len;
  char buffer[BUFFER_SIZE];

  if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    return -1;
  }

  memset(&servAddr, 0, sizeof(servAddr));
  memset(&clientAddr, 0, sizeof(clientAddr));

  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(PORT);

  if (bind(sockFd, (const struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
    perror("bind");
    close(sockFd);
    return -1;
  }

  printf("Server is listening on port %d...\n", PORT);

  while (1) {
    len = sizeof(clientAddr);

    int n = recvfrom(
        sockFd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &len);
    if (n < 0) {
      perror("recvfrom");
      continue;
    }

    buffer[n] = '\0';
    printf("Client sends: %s\n", buffer);

    modifyData(buffer);
    printf("Modified string: %s\n", buffer);

    if (sendto(
            sockFd,
            buffer,
            strlen(buffer),
            0,
            (const struct sockaddr*)&clientAddr,
            len) < 0) {
      perror("sendto");
    }

    printf("Reply sent to client. Waiting for the next one\n\n");
  }

  close(sockFd);
  return 0;
}