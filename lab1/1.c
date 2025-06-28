#include <stdio.h>

void intToBinary(unsigned int num) {
  unsigned int mask = 1 << (sizeof(unsigned int) * 8 - 1);
  
  while (mask != 0) {
    printf("%d", (mask & num) ? 1 : 0);
    mask >>= 1;
  }
  
  printf("\n");
}

int main() {
  unsigned int num = 0;
  
  printf("Write unsigned integer:\n");
  scanf("%u", &num);
  
  intToBinary(num);
  
  return 0;
}
