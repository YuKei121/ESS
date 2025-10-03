#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define SERVER_PORT 9090
#define MAX_CLIENTS 100
#define BUF_SIZE 1024

struct client {
  int ip;
  int port;
  int counter;
};

struct client clients[MAX_CLIENTS];
int sockFd;

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

void cleanup() {
  close(sockFd);
  exit(0);
}

void handleSignal(int sig) {
  if (sig == SIGINT) {
    printf("\nShutting down server...\n");
    cleanup();
  }
}

void formIpHeader(
    struct iphdr* ipHeader,
    int serverIp,
    int clientIp,
    int totalLength) {
  ipHeader->version = 4;
  ipHeader->ihl = 5;
  ipHeader->tos = 0;
  ipHeader->tot_len = htons(totalLength);
  ipHeader->id = htons(rand() % 65535);
  ipHeader->frag_off = 0;
  ipHeader->ttl = 64;
  ipHeader->protocol = IPPROTO_UDP;
  ipHeader->saddr = serverIp;
  ipHeader->daddr = clientIp;
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
    int servPort,
    const char* data,
    int dataLength,
    int clientIp,
    int clientPort) {
  struct iphdr* ipHeader = (struct iphdr*)packet;
  struct udphdr* udpHeader = (struct udphdr*)(packet + sizeof(struct iphdr));
  char* fullData = packet + sizeof(struct iphdr) + sizeof(struct udphdr);

  memcpy(fullData, data, dataLength);

  formIpHeader(
      ipHeader,
      inet_addr("127.0.0.1"),
      clientIp,
      sizeof(struct iphdr) + sizeof(struct udphdr) + dataLength);
  formUdpHeader(udpHeader, servPort, clientPort, dataLength);
}

int checkPacket(char* buf, int len, int portExpected) {
  if (len < sizeof(struct iphdr) + sizeof(struct udphdr)) {
    return -1;
  }

  struct iphdr* ipHeader = (struct iphdr*)buf;

  if (ipHeader->protocol != IPPROTO_UDP) {
    return -1;
  }
  if (ntohs(ipHeader->tot_len) > len) {
    return -1;
  }

  struct udphdr* udpHeader = (struct udphdr*)(buf + sizeof(struct iphdr));

  if (ntohs(udpHeader->dest) != portExpected) {
    return -1;
  }

  return len - sizeof(struct iphdr) - sizeof(struct udphdr);
}

struct client* findClient(int ip, int port) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].ip == ip && clients[i].port == port &&
        clients[i].counter > 0) {
      return &clients[i];
    }
  }
  return NULL;
}

struct client* addClient(int ip, int port) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].counter == 0) {
      clients[i].ip = ip;
      clients[i].port = port;
      clients[i].counter = 1;
      printf(
          "New client: %s:%d\n", inet_ntoa(*(struct in_addr*)&ip), ntohs(port));
      return &clients[i];
    }
  }
  return NULL;
}

void removeClient(int ip, int port) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].ip == ip && clients[i].port == port) {
      printf(
          "Client %s:%d exited with counter: %d\n",
          inet_ntoa(*(struct in_addr*)&ip),
          ntohs(port),
          clients[i].counter);
      memset(&clients[i], 0, sizeof(struct client));
      break;
    }
  }
}

int main() {
  signal(SIGINT, handleSignal);
  srand(time(NULL));

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

  printf("Server started on port %d\n", SERVER_PORT);

  char buf[BUF_SIZE];
  memset(clients, 0, sizeof(clients));

  while (1) {
    struct sockaddr_in clientAddr;
    int addrLength = sizeof(clientAddr);

    int len = recvfrom(
        sockFd, buf, BUF_SIZE, 0, (struct sockaddr*)&clientAddr, &addrLength);
    if (len < 0) {
      perror("recvfrom");
      continue;
    }

    int dataLength = checkPacket(buf, len, SERVER_PORT);
    if (dataLength < 0) {
      continue;
    }

    struct iphdr* ipHeader = (struct iphdr*)buf;
    struct udphdr* udpHeader = (struct udphdr*)(buf + sizeof(struct iphdr));

    char* data = (char*)(buf + sizeof(struct iphdr) + sizeof(struct udphdr));
    data[dataLength] = '\0';

    int clientIp = ipHeader->saddr;
    int clientPort = udpHeader->source;

    printf(
        "Received from %s:%d: %s\n",
        inet_ntoa(*(struct in_addr*)&clientIp),
        ntohs(clientPort),
        data);

    if (strcmp(data, "CLOSE") == 0) {
      removeClient(clientIp, clientPort);
      continue;
    }

    struct client* client = findClient(clientIp, clientPort);
    if (!client) {
      client = addClient(clientIp, clientPort);
      if (!client) {
        printf("Too many clients\n");
        continue;
      }
    }

    char reply[BUF_SIZE];
    int replyLength =
        snprintf(reply, sizeof(reply), "%s %d", data, client->counter);

    char packet[BUF_SIZE];
    formPacket(
        packet, htons(SERVER_PORT), reply, replyLength, clientIp, clientPort);

    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_addr = {.s_addr = clientIp},
        .sin_port = clientPort};

    int sent = sendto(
        sockFd,
        packet,
        sizeof(struct iphdr) + sizeof(struct udphdr) + replyLength,
        0,
        (struct sockaddr*)&dest_addr,
        sizeof(dest_addr));

    if (sent > 0) {
      printf("Sent reply: %s\n", reply);
      client->counter++;
    } else {
      perror("sendto");
    }
  }

  return 0;
}