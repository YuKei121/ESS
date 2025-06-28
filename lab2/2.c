#include <stdio.h>
#include <stdlib.h>

int main() {
  unsigned int size = 0;
  
  printf("Write matrix size:\n");
  scanf("%u", &size);
  
  int* arr = malloc(sizeof(unsigned int) * size);
  for (int i = 0; i < size; ++i) {
    arr[i] = i + 1;
    printf("%d ", arr[i]);
  }
  printf("\n");
  
  for (int i = size - 1; i >= 0; --i) {
    printf("%d ", arr[i]);
  }
  printf("\n");
  
  free(arr);
  
  return 0;
}
