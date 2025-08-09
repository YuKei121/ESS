#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  char* fifoPath = "customFIFO";
  mkfifo(fifoPath, 0666);
  int fd = open(fifoPath, O_WRONLY);
  write(fd, "Hi!", strlen("Hi!"));
  close(fd);
  return 0;
}