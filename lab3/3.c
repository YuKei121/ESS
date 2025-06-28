#include <stdio.h>
#include <stdlib.h>

int main() {
  int size = 10;
  int* arr = malloc(sizeof(int) * size);
  int* ptr = arr;
  
  for (int i = 0; i < size; ++i) {
    *ptr = i + 1;
    printf("%d ", *ptr);
    ++ptr;
  }
  printf("\n");
  
  return 0;
}
