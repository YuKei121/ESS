#include <stdio.h>

void intToBinary(int num) {
  unsigned int mask = 1 << (sizeof(int) * 8 - 1);
  
  while (mask != 0) {
    printf("%d", (mask & num) ? 1 : 0);
    mask >>= 1;
  }
  
  printf("\n");
}

int main() {
  int num = 0;
  
  printf("Write integer:\n");
  scanf("%d", &num);
  
  intToBinary(num);
  
  return 0;
}
