#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int sockFd;
  struct sockaddr_in servAddr;
  char message[] = "Hello, Server!";

  if ((sockFd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
    perror("socket");
    return -1;
  }

  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(PORT);
  servAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

  struct udphdr udpHeader;
  udpHeader.source = htons(12345);
  udpHeader.dest = htons(PORT);
  udpHeader.len = htons(sizeof(struct udphdr) + strlen(message));
  udpHeader.check = 0;

  char packet[sizeof(struct udphdr) + BUFFER_SIZE];
  memcpy(packet, &udpHeader, sizeof(struct udphdr));
  memcpy(packet + sizeof(struct udphdr), message, strlen(message));

  printf("Sending message: %s\n", message);

  if (sendto(
          sockFd,
          packet,
          sizeof(struct udphdr) + strlen(message),
          0,
          (struct sockaddr*)&servAddr,
          sizeof(servAddr)) < 0) {
    perror("sendto");
    close(sockFd);
    return -1;
  }

  printf("Packet with UDP header sent\n");

  char recvBuf[BUFFER_SIZE + 60];
  printf("Waiting for response from server...\n");

  while (1) {
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    int n = recvfrom(
        sockFd,
        recvBuf,
        sizeof(recvBuf),
        0,
        (struct sockaddr*)&from_addr,
        &from_len);

    if (n < 0) {
      perror("recvfrom");
      continue;
    }

    // ip header check
    struct ip* ipHeader = (struct ip*)recvBuf;
    int ipLength = ipHeader->ip_hl * 4;

    if (ipHeader->ip_p != IPPROTO_UDP) {
      continue;
    }

    // udp header check
    struct udphdr* udpHeader = (struct udphdr*)(recvBuf + ipLength);

    // port check
    if (ntohs(udpHeader->dest) == 12345) {
      char* replyData = recvBuf + ipLength + sizeof(struct udphdr);
      int replyDataLength = n - ipLength - sizeof(struct udphdr);

      if (replyDataLength > 0) {
        replyData[replyDataLength] = '\0';
        printf("Server says: %s\n", replyData);
        break;
      }
    }
  }

  close(sockFd);
  return 0;
}