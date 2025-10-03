#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9090
#define BUF_SIZE 1024

int sockFd;
int clientPort = 0;

unsigned short csum(void* b, int len) {
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

void formIpHeader(
    struct iphdr* ipHeader,
    int clientIp,
    int serverIp,
    int totalLength) {
  ipHeader->version = 4;
  ipHeader->ihl = 5;
  ipHeader->tos = 0;
  ipHeader->tot_len = htons(totalLength);
  ipHeader->id = htons(rand() % 65535);
  ipHeader->frag_off = 0;
  ipHeader->ttl = 64;
  ipHeader->protocol = IPPROTO_UDP;
  ipHeader->saddr = clientIp;
  ipHeader->daddr = serverIp;
  ipHeader->check = csum(ipHeader, sizeof(struct iphdr));
}

void formUdpHeader(
    struct udphdr* udpHeader,
    int clientPort,
    int serverPort,
    int dataLength) {
  udpHeader->source = clientPort;
  udpHeader->dest = serverPort;
  udpHeader->len = htons(sizeof(struct udphdr) + dataLength);
  udpHeader->check = 0;
}

void formPacket(
    char* packet,
    int clientPort,
    const char* data,
    int dataLength) {
  struct iphdr* ipHeader = (struct iphdr*)packet;
  struct udphdr* udpHeader = (struct udphdr*)(packet + sizeof(struct iphdr));
  char* payload = packet + sizeof(struct iphdr) + sizeof(struct udphdr);

  memcpy(payload, data, dataLength);

  formIpHeader(
      ipHeader,
      inet_addr("127.0.0.1"),
      inet_addr(SERVER_IP),
      sizeof(struct iphdr) + sizeof(struct udphdr) + dataLength);
  formUdpHeader(udpHeader, clientPort, htons(SERVER_PORT), dataLength);
}

int checkPacket(char* buf, int len, int portExpected) {
  if (len < sizeof(struct iphdr) + sizeof(struct udphdr)) {
    return -1;
  }

  struct iphdr* ipHeader = (struct iphdr*)buf;

  if (ipHeader->protocol != IPPROTO_UDP) {
    return -1;
  }
  if (ipHeader->saddr != inet_addr(SERVER_IP)) {
    return -1;
  }
  if (ntohs(ipHeader->tot_len) > len) {
    return -1;
  }

  struct udphdr* udpHeader = (struct udphdr*)(buf + sizeof(struct iphdr));

  if (ntohs(udpHeader->dest) != portExpected) {
    return -1;
  }
  if (ntohs(udpHeader->source) != SERVER_PORT) {
    return -1;
  }

  return len - sizeof(struct iphdr) - sizeof(struct udphdr);
}

void closingMessage() {
  char packet[BUF_SIZE];
  formPacket(packet, clientPort, "CLOSE", 5);

  struct sockaddr_in servAddr = {
      .sin_family = AF_INET,
      .sin_addr = {.s_addr = inet_addr(SERVER_IP)},
      .sin_port = htons(SERVER_PORT)};

  sendto(
      sockFd,
      packet,
      sizeof(struct iphdr) + sizeof(struct udphdr) + 5,
      0,
      (struct sockaddr*)&servAddr,
      sizeof(servAddr));

  printf("Sent closing message to server.\n");
}

void cleanup() {
  closingMessage();
  close(sockFd);
  exit(0);
}

void handleSignal(int sig) {
  if (sig == SIGINT) {
    printf("Shutting down client...\n");
    cleanup();
  }
}

int getPort() {
  int tempSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (tempSock < 0) {
    return htons(0);
  }

  struct sockaddr_in addr = {
      .sin_family = AF_INET, .sin_addr = {.s_addr = INADDR_ANY}, .sin_port = 0};

  if (bind(tempSock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    close(tempSock);
    return htons(0);
  }

  int len = sizeof(addr);
  getsockname(tempSock, (struct sockaddr*)&addr, &len);
  int port = addr.sin_port;

  close(tempSock);
  return port;
}

int main() {
  signal(SIGINT, handleSignal);
  srand(time(NULL));

  clientPort = getPort();
  if (clientPort == 0) {
    printf("getPort() failed, getting random port...\n");
    clientPort = htons(1024 + (rand() % (65535 - 1024)));
  }

  printf("Client using port: %d\n", ntohs(clientPort));

  sockFd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if (sockFd < 0) {
    perror("socket");
    return -1;
  }

  int flag = 1;
  if (setsockopt(sockFd, IPPROTO_IP, IP_HDRINCL, &flag, sizeof(flag)) < 0) {
    perror("setsockopt");
    close(sockFd);
    return -1;
  }

  printf("Client will send messages to server %s:%d\n", SERVER_IP, SERVER_PORT);

  char buf[BUF_SIZE];

  while (1) {
    printf("Enter message: ");
    fflush(stdout);

    char message[BUF_SIZE];
    if (!fgets(message, sizeof(message), stdin)) {
      break;
    }
    message[strcspn(message, "\n")] = '\0';

    if (strlen(message) == 0) {
      continue;
    }

    char packet[BUF_SIZE];
    formPacket(packet, clientPort, message, strlen(message));

    struct sockaddr_in servAddr = {
        .sin_family = AF_INET,
        .sin_addr = {.s_addr = inet_addr(SERVER_IP)},
        .sin_port = htons(SERVER_PORT)};

    int sent = sendto(
        sockFd,
        packet,
        sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(message),
        0,
        (struct sockaddr*)&servAddr,
        sizeof(servAddr));

    if (sent < 0) {
      perror("sendto");
      continue;
    }

    printf("Message sent, waiting for reply...\n");

    fd_set readFds;
    int val;

    FD_ZERO(&readFds);
    FD_SET(sockFd, &readFds);

    val = select(sockFd + 1, &readFds, NULL, NULL, NULL);

    if (val == -1) {
      perror("select");
    } else {
      while (1) {
        int len = recvfrom(sockFd, buf, BUF_SIZE, 0, NULL, NULL);
        if (len <= 0)
          break;

        int dataLength = checkPacket(buf, len, ntohs(clientPort));
        if (dataLength > 0) {
          char* reply_data = buf + sizeof(struct iphdr) + sizeof(struct udphdr);
          reply_data[dataLength] = '\0';
          printf("Server reply: %s\n", reply_data);
          break;
        }
      }
    }
  }

  cleanup();
  return 0;
}