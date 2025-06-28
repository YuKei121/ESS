#include <stdio.h>
#include <stdlib.h>

int main() {
  unsigned int size = 0;
  unsigned int vars = 0;
  
  printf("Write matrix size:\n");
  scanf("%u", &size);
  
  vars = size * size;
  
  int* arr = malloc(sizeof(unsigned int) * vars);
  for (int i = 0; i < vars; ++i) {
    arr[i] = i + 1;
  }
  
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      printf("%u ", arr[i * size + j]);
    }
    printf("\n");
  }
  
  free(arr);
  
  return 0;
}
