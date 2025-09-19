#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MULTICAST_PORT 5555
#define MULTICAST_GROUP "224.0.0.100"
#define BUFFER_SIZE 1024

int main() {
  int fd;
  struct sockaddr_in multicast_addr;
  char buffer[BUFFER_SIZE];

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd == -1) {
    perror("socket creation failed");
    return -1;
  }
  int TTL = 2;

  if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &TTL, sizeof(TTL)) == -1) {
    perror("setsockopt IP_MULTICAST_TTL failed");
    close(fd);
    return -1;
  }

  memset(&multicast_addr, 0, sizeof(multicast_addr));
  multicast_addr.sin_family = AF_INET;
  multicast_addr.sin_port = htons(MULTICAST_PORT);
  if (inet_pton(AF_INET, MULTICAST_GROUP, &multicast_addr.sin_addr) <= 0) {
    perror("invalid multicast address");
    close(fd);
    return -1;
  }

  printf(
      "Multicast server started. Sending to %s:%d\n",
      MULTICAST_GROUP,
      MULTICAST_PORT);

  int message_count = 0;
  while (1) {
    snprintf(
        buffer,
        BUFFER_SIZE,
        "Multicast message %d. Random symbols: %c%c%c",
        ++message_count,
        'A' + rand() % 26,
        'a' + rand() % 26,
        '0' + rand() % 10);

    if (sendto(
            fd,
            buffer,
            strlen(buffer),
            0,
            (struct sockaddr*)&multicast_addr,
            sizeof(multicast_addr)) == -1) {
      perror("sendto failed");
      continue;
    }

    printf("Sent: %s\n", buffer);
    sleep(2);
  }

  close(fd);
  return 0;
}