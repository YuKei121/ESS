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
  struct sockaddr_in local_addr, multicast_addr;
  struct ip_mreq mreq;
  socklen_t addr_len = sizeof(multicast_addr);
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

  memset(&local_addr, 0, sizeof(local_addr));
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(MULTICAST_PORT);
  local_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1) {
    perror("bind failed");
    close(fd);
    return -1;
  }

  mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
  mreq.imr_interface.s_addr = INADDR_ANY;

  if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) ==
      -1) {
    perror("setsockopt IP_ADD_MEMBERSHIP failed");
    close(fd);
    return -1;
  }

  printf(
      "Multicast client started. Joined group %s:%d\n",
      MULTICAST_GROUP,
      MULTICAST_PORT);

  while (1) {
    bytesReceived = recvfrom(
        fd,
        buffer,
        BUFFER_SIZE,
        0,
        (struct sockaddr*)&multicast_addr,
        &addr_len);
    if (bytesReceived == -1) {
      perror("recvfrom failed");
      continue;
    }

    buffer[bytesReceived] = '\0';
    printf(
        "Received from %s:%d: %s\n",
        inet_ntoa(multicast_addr.sin_addr),
        ntohs(multicast_addr.sin_port),
        buffer);
  }

  setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
  close(fd);
  return 0;
}