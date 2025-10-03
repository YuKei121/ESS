#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SERVER_IP "192.168.1.38"
#define CLIENT_IP "192.168.1.36"
#define PORT 8080
#define CLIENT_PORT 12345
#define BUFFER_SIZE 1024
#define INTERFACE "wlp1s0"

// spent a lot of time because of this
#pragma pack(push, 1)
struct packet {
  struct ethhdr ethernet;
  struct iphdr ipHeader;
  struct udphdr udpHeader;
  char data[BUFFER_SIZE];
};
#pragma pack(pop)

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

void getMacInterface(const char* interface, unsigned char* mac) {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  struct ifreq ifr;

  strcpy(ifr.ifr_name, interface);
  if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    printf(
        "getMacInterface, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
        mac[0],
        mac[1],
        mac[2],
        mac[3],
        mac[4],
        mac[5]);
  }
  close(fd);
}

void getDestinationMac(unsigned char* mac) {
  memset(mac, 0xFF, 6);
}

int main() {
  int sockFd;
  struct sockaddr_ll servAddr;
  char message[] = "Hello, Server!";

  if ((sockFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
    perror("socket");
    return -1;
  }

  struct ifreq ifr;
  strcpy(ifr.ifr_name, INTERFACE);
  if (ioctl(sockFd, SIOCGIFINDEX, &ifr) < 0) {
    perror("ioctl");
    close(sockFd);
    return -1;
  }

  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sll_family = AF_PACKET;
  servAddr.sll_ifindex = ifr.ifr_ifindex;
  servAddr.sll_protocol = htons(ETH_P_IP);

  struct packet pak;
  memset(&pak, 0, sizeof(pak));

  // ethernet header
  unsigned char src_mac[6], dst_mac[6];
  getMacInterface(INTERFACE, src_mac);
  getDestinationMac(dst_mac);
  memcpy(pak.ethernet.h_dest, dst_mac, 6);
  memcpy(pak.ethernet.h_source, src_mac, 6);
  pak.ethernet.h_proto = htons(ETH_P_IP);

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
  pak.ipHeader.saddr = inet_addr(CLIENT_IP);
  pak.ipHeader.daddr = inet_addr(SERVER_IP);
  pak.ipHeader.check = checksum(&pak.ipHeader, sizeof(pak.ipHeader));

  // udp header
  pak.udpHeader.source = htons(CLIENT_PORT);
  pak.udpHeader.dest = htons(PORT);
  pak.udpHeader.len = htons(sizeof(struct udphdr) + strlen(message));
  pak.udpHeader.check = 0;

  strcpy(pak.data, message);

  printf("Sending message: %s\n", message);
  printf(
      "Ethernet header: dest=%02x:%02x:%02x:%02x:%02x:%02x, "
      "src=%02x:%02x:%02x:%02x:%02x:%02x\n",
      pak.ethernet.h_dest[0],
      pak.ethernet.h_dest[1],
      pak.ethernet.h_dest[2],
      pak.ethernet.h_dest[3],
      pak.ethernet.h_dest[4],
      pak.ethernet.h_dest[5],
      pak.ethernet.h_source[0],
      pak.ethernet.h_source[1],
      pak.ethernet.h_source[2],
      pak.ethernet.h_source[3],
      pak.ethernet.h_source[4],
      pak.ethernet.h_source[5]);

  int pakLen = sizeof(struct ethhdr) + ntohs(pak.ipHeader.tot_len);
  if (sendto(
          sockFd,
          &pak,
          pakLen,
          0,
          (struct sockaddr*)&servAddr,
          sizeof(servAddr)) < 0) {
    perror("sendto");
    close(sockFd);
    return -1;
  }
  printf("Waiting for response from server...\n");

  char recvBuf
      [BUFFER_SIZE + sizeof(struct ethhdr) + sizeof(struct iphdr) +
       sizeof(struct udphdr)];

  while (1) {
    int n = recvfrom(sockFd, recvBuf, sizeof(recvBuf), 0, NULL, NULL);
    if (n < 0) {
      perror("recvfrom");
      continue;
    }

    // ethernet check
    struct ethhdr* tempEthernetHeader = (struct ethhdr*)recvBuf;

    if (ntohs(tempEthernetHeader->h_proto) != ETH_P_IP) {
      continue;
    }

    // ip header check
    struct iphdr* tempIpHeader =
        (struct iphdr*)(recvBuf + sizeof(struct ethhdr));
    int tempIpHeaderLen = tempIpHeader->ihl * 4;

    if (tempIpHeader->protocol != IPPROTO_UDP) {
      continue;
    }

    if (tempIpHeader->daddr != inet_addr(CLIENT_IP)) {
      continue;
    }

    // udp header check
    struct udphdr* tempUdpHeader =
        (struct udphdr*)(recvBuf + sizeof(struct ethhdr) + tempIpHeaderLen);

    // port check
    if (ntohs(tempUdpHeader->dest) != CLIENT_PORT) {
      continue;
    }

    char* replyData = recvBuf + sizeof(struct ethhdr) + tempIpHeaderLen +
                      sizeof(struct udphdr);
    int replyDataLength =
        n - sizeof(struct ethhdr) - tempIpHeaderLen - sizeof(struct udphdr);

    if (replyDataLength > 0) {
      replyData[replyDataLength] = '\0';
      printf("Server says: %s\n", replyData);
      break;
    }
  }

  close(sockFd);
  return 0;
}
