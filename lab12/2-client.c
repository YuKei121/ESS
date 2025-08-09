#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  char* fifoPath = "customFIFO";
  char buf[100];
  mkfifo(fifoPath, 0666);
  int fd = open(fifoPath, O_RDONLY);
  if (read(fd, buf, sizeof(buf)) > 0) {
    printf("Recieved: %s\n", buf);
  }
  close(fd);
  unlink(fifoPath);
  return 0;
}