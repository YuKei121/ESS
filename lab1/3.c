#include <stdio.h>

void intToBinary(int num) {
  unsigned int mask = 1 << (sizeof(int) * 8 - 1);
  
  short count = 0;
  while (mask != 0) {
    count += (mask & num) ? 1 : 0;
    mask >>= 1;
  }
  
  printf("%d\n", count);
}

int main() {
  int num = 0;
  
  printf("Write integer:\n");
  scanf("%d", &num);
  
  intToBinary(num);
  
  return 0;
}
