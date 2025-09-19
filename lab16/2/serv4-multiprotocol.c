#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 10
#define BACKLOG 5

void handleTCP(int client_fd, struct sockaddr_in* client_addr) {
  char buffer[BUFFER_SIZE];
  char timeStr[50];
  time_t curTime;
  ssize_t bytesReceived;

  bytesReceived = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
  if (bytesReceived <= 0) {
    close(client_fd);
    return;
  }

  buffer[bytesReceived] = '\0';
  printf(
      "TCP: Received from %s:%d: %s\n",
      inet_ntoa(client_addr->sin_addr),
      ntohs(client_addr->sin_port),
      buffer);

  if (strcmp(buffer, "time") == 0) {
    curTime = time(NULL);
    ctime_r(&curTime, timeStr);
    send(client_fd, timeStr, strlen(timeStr), 0);
    printf("TCP: Sent time to client\n");
  }

  close(client_fd);
}

void handleUDP(int udpFd, struct sockaddr_in* client_addr) {
  char buffer[BUFFER_SIZE];
  char timeStr[50];
  time_t curTime;
  socklen_t clientLen = sizeof(*client_addr);
  ssize_t bytesReceived;

  bytesReceived = recvfrom(
      udpFd,
      buffer,
      BUFFER_SIZE - 1,
      0,
      (struct sockaddr*)client_addr,
      &clientLen);
  if (bytesReceived <= 0) {
    return;
  }

  buffer[bytesReceived] = '\0';
  printf(
      "UDP: Received from %s:%d: %s\n",
      inet_ntoa(client_addr->sin_addr),
      ntohs(client_addr->sin_port),
      buffer);

  if (strcmp(buffer, "time") == 0) {
    curTime = time(NULL);
    ctime_r(&curTime, timeStr);
    sendto(
        udpFd,
        timeStr,
        strlen(timeStr),
        0,
        (struct sockaddr*)client_addr,
        clientLen);
    printf("UDP: Sent time to client\n");
  }
}

int main() {
  int tcpFd, udpFd, epollFd;
  struct sockaddr_in server_addr;
  struct epoll_event event, events[MAX_EVENTS];
  int nfds, i;

  tcpFd = socket(AF_INET, SOCK_STREAM, 0);
  if (tcpFd == -1) {
    perror("TCP socket creation failed");
    return -1;
  }

  udpFd = socket(AF_INET, SOCK_DGRAM, 0);
  if (udpFd == -1) {
    perror("UDP socket creation failed");
    close(tcpFd);
    return -1;
  }

  int reuse = 1;
  setsockopt(tcpFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  setsockopt(udpFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(tcpFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("TCP bind failed");
    close(tcpFd);
    close(udpFd);
    return -1;
  }

  if (bind(udpFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("UDP bind failed");
    close(tcpFd);
    close(udpFd);
    return -1;
  }

  if (listen(tcpFd, BACKLOG) == -1) {
    perror("listen failed");
    close(tcpFd);
    close(udpFd);
    return -1;
  }

  epollFd = epoll_create1(0);
  if (epollFd == -1) {
    perror("epoll_create1 failed");
    close(tcpFd);
    close(udpFd);
    return -1;
  }

  event.events = EPOLLIN;
  event.data.fd = tcpFd;
  if (epoll_ctl(epollFd, EPOLL_CTL_ADD, tcpFd, &event) == -1) {
    perror("epoll_ctl TCP failed");
    close(tcpFd);
    close(udpFd);
    close(epollFd);
    return -1;
  }

  event.events = EPOLLIN;
  event.data.fd = udpFd;
  if (epoll_ctl(epollFd, EPOLL_CTL_ADD, udpFd, &event) == -1) {
    perror("epoll_ctl UDP failed");
    close(tcpFd);
    close(udpFd);
    close(epollFd);
    return -1;
  }

  printf("Multiprotocol server (TCP+UDP) listening on port %d\n", PORT);

  while (1) {
    nfds = epoll_wait(epollFd, events, MAX_EVENTS, -1);
    if (nfds == -1) {
      perror("epoll_wait failed");
      break;
    }

    for (i = 0; i < nfds; i++) {
      if (events[i].data.fd == tcpFd) {
        struct sockaddr_in client_addr;
        socklen_t clientLen = sizeof(client_addr);
        int client_fd =
            accept(tcpFd, (struct sockaddr*)&client_addr, &clientLen);

        if (client_fd == -1) {
          perror("accept failed");
          continue;
        }

        printf(
            "TCP client connected: %s:%d\n",
            inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port));

        handleTCP(client_fd, &client_addr);

      } else if (events[i].data.fd == udpFd) {
        struct sockaddr_in client_addr;
        handleUDP(udpFd, &client_addr);
      }
    }
  }

  close(tcpFd);
  close(udpFd);
  close(epollFd);
  return 0;
}