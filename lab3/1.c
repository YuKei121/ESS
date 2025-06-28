#include <stdio.h>

void changeThirdByte(int num, char byte) {
  char *ptr = &num;
  ptr += 2;
  *ptr = byte;
  printf("%d\n", num);
}

int main() {
  int num = 0;
  char byte = 0;
  char in[15];
  
  printf("Input a number and a byte (example: 121 5): ");
  fgets(in, sizeof(in), stdin);
  sscanf(in, "%u %hhu", &num, &byte);
  
  changeThirdByte(num, byte);
  
  return 0;
}
