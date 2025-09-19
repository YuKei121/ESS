#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/local_stream_socket"
#define BUFFER_SIZE 1024

int main() {
  int fd;
  struct sockaddr_un server_addr;
  char buffer[BUFFER_SIZE];
  ssize_t bytesReceived;

  fd = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (fd == -1) {
    perror("socket creation failed");
    return -1;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sun_family = AF_LOCAL;
  strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

  if (connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("connect failed");
    close(fd);
    return -1;
  }

  const char* message = "hello!";
  if (write(fd, message, strlen(message)) == -1) {
    perror("write failed");
    close(fd);
    return -1;
  }

  printf("Sent to server: %s\n", message);

  bytesReceived = read(fd, buffer, BUFFER_SIZE - 1);
  if (bytesReceived == -1) {
    perror("read failed");
    close(fd);
    return -1;
  }

  buffer[bytesReceived] = '\0';
  printf("Received from server: %s\n", buffer);

  close(fd);
  return 0;
}