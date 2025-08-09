#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  int fd = open("test.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
  // char in[] = "Was it a cat I saw";
  char in[] = "String from file";
  size_t size = sizeof(in);
  char out[size];
  write(fd, in, size);
  for (size_t i = 0; i < size; i++) {
    lseek(fd, -2 - i, SEEK_END);
    read(fd, &out[i], 1);
  }
  out[size - 1] = '\0';
  printf("%s\n", in);
  printf("%s\n", out);
  close(fd);
  return 0;
}