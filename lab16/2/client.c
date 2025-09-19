#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

// TCP by default. Add 'udp' to change client to UDP. Example: ./client udp
int main(int argc, char* argv[]) {
  int fd;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];
  int use_tcp = 1;

  if (argc >= 2 && strcmp(argv[1], "udp") == 0) {
    use_tcp = 0;
  }

  if (use_tcp) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
  } else {
    fd = socket(AF_INET, SOCK_DGRAM, 0);
  }

  if (fd == -1) {
    perror("socket creation failed");
    return -1;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
    perror("invalid address");
    close(fd);
    return -1;
  }

  if (use_tcp) {
    if (connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) ==
        -1) {
      perror("connect failed");
      close(fd);
      return -1;
    }
    printf("Connected to TCP server %s:%d\n", SERVER_IP, PORT);
  } else {
    printf("Ready to send to UDP server %s:%d\n", SERVER_IP, PORT);
  }

  const char* request = "time";
  if (use_tcp) {
    if (send(fd, request, strlen(request), 0) == -1) {
      perror("send failed");
      close(fd);
      return -1;
    }
  } else {
    if (sendto(
            fd,
            request,
            strlen(request),
            0,
            (struct sockaddr*)&server_addr,
            sizeof(server_addr)) == -1) {
      perror("sendto failed");
      close(fd);
      return -1;
    }
  }

  printf("Sent request: %s\n", request);

  ssize_t bytesReceived;
  if (use_tcp) {
    bytesReceived = recv(fd, buffer, BUFFER_SIZE - 1, 0);
  } else {
    socklen_t addr_len = sizeof(server_addr);
    bytesReceived = recvfrom(
        fd,
        buffer,
        BUFFER_SIZE - 1,
        0,
        (struct sockaddr*)&server_addr,
        &addr_len);
  }

  if (bytesReceived == -1) {
    perror("recv failed");
    close(fd);
    return -1;
  }

  buffer[bytesReceived] = '\0';
  printf("Current time: %s\n", buffer);

  close(fd);
  return 0;
}