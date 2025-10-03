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

struct packet {
  struct iphdr ipHeader;
  struct udphdr updHeader;
  char data[BUFFER_SIZE];
};

unsigned short checksum(void* b, int len) {
  unsigned short* buf = b;
  unsigned int sum = 0;
  unsigned short result;

  for (sum = 0; len > 1; len -= 2) {
    sum += *buf++;
  }

  if (len == 1) {
    sum += *(unsigned char*)buf;
  }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  result = ~sum;

  return result;
}

int main() {
  int sockFd;
  struct sockaddr_in servAddr;
  char message[] = "Hello, Server!";

  if ((sockFd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
    perror("socket");
    return -1;
  }

  int flag = 1;
  if (setsockopt(sockFd, IPPROTO_IP, IP_HDRINCL, &flag, sizeof(flag)) < 0) {
    perror("setsockopt");
    close(sockFd);
    return -1;
  }

  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(PORT);
  servAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

  struct packet pak;
  memset(&pak, 0, sizeof(pak));

  // ip header
  pak.ipHeader.ihl = 5;
  pak.ipHeader.version = 4;
  pak.ipHeader.tos = 0;
  pak.ipHeader.tot_len =
      htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(message));
  pak.ipHeader.id = htons(54321);
  pak.ipHeader.frag_off = 0;
  pak.ipHeader.ttl = 255;
  pak.ipHeader.protocol = IPPROTO_UDP;
  pak.ipHeader.saddr = inet_addr("127.0.0.1");
  pak.ipHeader.daddr = servAddr.sin_addr.s_addr;
  pak.ipHeader.check = checksum(&pak.ipHeader, sizeof(pak.ipHeader));

  // udp header
  pak.updHeader.source = htons(12345);
  pak.updHeader.dest = htons(PORT);
  pak.updHeader.len = htons(sizeof(struct udphdr) + strlen(message));
  pak.updHeader.check = 0;

  strcpy(pak.data, message);

  if (sendto(
          sockFd,
          &pak,
          ntohs(pak.ipHeader.tot_len),
          0,
          (struct sockaddr*)&servAddr,
          sizeof(servAddr)) < 0) {
    perror("sendto");
    close(sockFd);
    return -1;
  }

  printf("Message sent: %s\n", message);
  printf("Waiting for reply from server...\n");

  char recvBuf[BUFFER_SIZE + sizeof(struct iphdr) + sizeof(struct udphdr)];
  struct sockaddr_in fromAddr;
  socklen_t fromLength = sizeof(fromAddr);

  while (1) {
    int n = recvfrom(
        sockFd,
        recvBuf,
        sizeof(recvBuf),
        0,
        (struct sockaddr*)&fromAddr,
        &fromLength);

    if (n < 0) {
      perror("recvfrom");
      continue;
    }

    // ip header check
    struct iphdr* tempIpHeader = (struct iphdr*)recvBuf;
    int tempIpHeaderLen = tempIpHeader->ihl * 4;

    if (tempIpHeader->protocol != IPPROTO_UDP) {
      continue;
    }

    // udp header check
    struct udphdr* tempUdpHeader = (struct udphdr*)(recvBuf + tempIpHeaderLen);

    // port check
    if (ntohs(tempUdpHeader->dest) != 12345) {
      continue;
    }

    // serv port check
    if (ntohs(tempUdpHeader->source) != PORT) {
      continue;
    }

    char* replyData = recvBuf + tempIpHeaderLen + sizeof(struct udphdr);
    int replyDataLength = n - tempIpHeaderLen - sizeof(struct udphdr);

    if (replyDataLength > 0) {
      replyData[replyDataLength] = '\0';
      printf("Server says: %s\n", replyData);
      break;
    }
  }

  close(sockFd);
  return 0;
}
