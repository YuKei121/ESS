#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/local_dgram_socket"
#define SERVER_PATH "/tmp/local_dgram_socket"
#define CLIENT_PATH "/tmp/local_dgram_client"
#define BUFFER_SIZE 1024

int main() {
  int fd;
  struct sockaddr_un server_addr, client_addr;
  char buffer[BUFFER_SIZE];
  ssize_t bytesReceived;

  fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (fd == -1) {
    perror("socket creation failed");
    return -1;
  }

  remove(CLIENT_PATH);

  memset(&client_addr, 0, sizeof(client_addr));
  client_addr.sun_family = AF_LOCAL;
  strncpy(client_addr.sun_path, CLIENT_PATH, sizeof(client_addr.sun_path) - 1);

  if (bind(fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
    perror("client bind failed");
    close(fd);
    return -1;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sun_family = AF_LOCAL;
  strncpy(server_addr.sun_path, SERVER_PATH, sizeof(server_addr.sun_path) - 1);

  const char* message = "hello!";
  if (sendto(
          fd,
          message,
          strlen(message),
          0,
          (struct sockaddr*)&server_addr,
          sizeof(server_addr)) == -1) {
    perror("sendto failed");
    close(fd);
    return -1;
  }

  printf("Sent to server: %s\n", message);

  bytesReceived = recvfrom(fd, buffer, BUFFER_SIZE, 0, NULL, NULL);
  if (bytesReceived == -1) {
    perror("recvfrom failed");
    close(fd);
    return -1;
  }

  buffer[bytesReceived] = '\0';
  printf("Received from server: %s\n", buffer);

  close(fd);
  remove(CLIENT_PATH);
  return 0;
}