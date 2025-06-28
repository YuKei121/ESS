#include <stdio.h>

void changeThirdByte(unsigned int num, unsigned char byte) {
  unsigned int mask = 1 << (sizeof(unsigned int) * 8 - 1);
  
  unsigned int newNum = num & ~(255 << 16);
  newNum |= (byte << 16);
  while (mask != 0) {
    printf("%d", (mask & newNum) ? 1 : 0);
    mask >>= 1;
  }
  
  printf("\n%d\n", newNum);
}

int main() {
  unsigned int num = 0;
  unsigned char byte = 0;
  char in[15];
  
  printf("Input a number and a byte (example: 121 5): ");
  fgets(in, sizeof(in), stdin);
  sscanf(in, "%u %hhu", &num, &byte);
  
  changeThirdByte(num, byte);
  
  return 0;
}
