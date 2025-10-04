#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

struct ioctl_arg {
  unsigned int val;
};

#define IOC_MAGIC '\x66'
#define IOCTL_VALSET _IOW(IOC_MAGIC, 0, struct ioctl_arg)
#define IOCTL_VALGET _IOR(IOC_MAGIC, 1, struct ioctl_arg)
#define IOCTL_VALGET_NUM _IOR(IOC_MAGIC, 2, int)
#define IOCTL_VALSET_NUM _IOW(IOC_MAGIC, 3, int)

int main() {
  int fd;
  struct ioctl_arg data;
  int number;

  fd = open("/dev/ioctltest", O_RDWR);
  if (fd < 0) {
    perror("Failed to open the device");
    return -1;
  }

  data.val = 0xABCD;
  if (ioctl(fd, IOCTL_VALSET, &data) < 0) {
    perror("IOCTL_VALSET failed");
  }

  data.val = 0;
  if (ioctl(fd, IOCTL_VALGET, &data) < 0) {
    perror("IOCTL_VALGET failed");
  } else {
    printf("Got value from driver: 0x%x\n", data.val);
  }

  number = 42;
  if (ioctl(fd, IOCTL_VALSET_NUM, number) < 0) {
    perror("IOCTL_VALSET_NUM failed");
  }

  number = 0;
  if (ioctl(fd, IOCTL_VALGET_NUM, &number) < 0) {
    perror("IOCTL_VALGET_NUM failed");
  } else {
    printf("Got number from driver: %d\n", number);
  }

  char buf[32];
  ssize_t bytesRead = read(fd, buf, sizeof(buf));
  if (bytesRead > 0) {
    printf("Read %zd bytes from device\n", bytesRead);
  }

  close(fd);
  return 0;
}
