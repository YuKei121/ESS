#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BROADCAST_PORT 4444
#define BROADCAST_IP "255.255.255.255"
#define BUFFER_SIZE 1024

int main() {
  int fd;
  struct sockaddr_in broadcast_addr;
  int broadcastOn = 1;
  char buffer[BUFFER_SIZE];

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd == -1) {
    perror("socket creation failed");
    return -1;
  }

  if (setsockopt(
          fd, SOL_SOCKET, SO_BROADCAST, &broadcastOn, sizeof(broadcastOn)) ==
      -1) {
    perror("setsockopt SO_BROADCAST failed");
    close(fd);
    return -1;
  }

  memset(&broadcast_addr, 0, sizeof(broadcast_addr));
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_port = htons(BROADCAST_PORT);
  if (inet_pton(AF_INET, BROADCAST_IP, &broadcast_addr.sin_addr) <= 0) {
    perror("invalid broadcast address");
    close(fd);
    return -1;
  }

  printf(
      "Broadcast server started. Sending to %s:%d\n",
      BROADCAST_IP,
      BROADCAST_PORT);

  int messageCount = 0;
  while (1) {
    snprintf(
        buffer,
        BUFFER_SIZE,
        "Broadcast message %d. Random symbols: %c%c%c",
        ++messageCount,
        'A' + rand() % 26,
        'a' + rand() % 26,
        '0' + rand() % 10);

    if (sendto(
            fd,
            buffer,
            strlen(buffer),
            0,
            (struct sockaddr*)&broadcast_addr,
            sizeof(broadcast_addr)) == -1) {
      perror("sendto failed");
      continue;
    }

    printf("Sent: %s\n", buffer);
    sleep(2);
  }

  close(fd);
  return 0;
}